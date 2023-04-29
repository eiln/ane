
# Tinygrad Corrections

Correcting incorrect information in [tinygrad](https://github.com/geohot/tinygrad/tree/59d0d168cd820426723218e662f8930770266415/accel/ane).

>Tinygrad quotes are quoted like this


## Command

>At its core, it runs through 0x300 ops in an hwx file

[Systems and Methods For Task Switching in Neural Network Processor [1]](https://patentimages.storage.googleapis.com/f5/fd/4b/ba09d9f878657f/US20190340014A1.pdf) introduces the microsequence/command sequence equivalent for the ANE:

"""Each instance of address data 1204A through 1204N (collectively or individually referred to as “address data") defines an address and data payload pair used to program the components of the neural processor circuit. For example, each instance of address data includes register data defining the data payload, a register address defining a destination memory location of the neural processing circuit for receiving the register data, and a register count defining a number of consecutive memory locations (e.g., registers) to be written with the register data.
""" [[1]](https://patentimages.storage.googleapis.com/f5/fd/4b/ba09d9f878657f/US20190340014A1.pdf)

With a diagram that resembles:

	--------------------------------
	[ Register ][ Register Address ]
	[ Count    ][                  ]
	--------------------------------
	[        Register Data         ]
	--------------------------------

**Translated:** We have some register values that need to be written to their designated register address. Thankfully, most of them are at consecutive addresses (e.g. value 0xCAFE at address 0x12130 and 0xBABE at 0x12134, the next u32 spot available at +0x4 up). So we store them as groups of consecutive streams deliminated by a special code that says 1) the number of values in this stream 2) the starting address for this stream. That way, we don't double store the address and can iterate over the stream of values quickly. Specifically, our u32 code packs `[Register Count][Register Address]` at `[31:24][23:0]`. I hope GPT cucks patent lawyers btw.

Here are the streams:

| Kernel     | Common     | Src        | L2         | Planar     | Neural     | Dst        |
|------------|------------|------------|------------|------------|------------|------------|
| 0xf401f800 | 0x3c000000 | 0x6c013800 | 0x44004800 | 0x0c008800 | 0x1000c800 | 0x18017800 |

For the "Kernel" field, count is `0xf401f800 >> 24 == 0xf4` and address is `0xf401f800 & 0xffffff == 0x1f800`. Assuming there exists a 0x28 sized header for other stuff, the total size of the command is then:

	0xf4 + 0x3c + 0x6c + 0x44 + 0x0c + 0x10 + 0x18 + (0x8 * 7) + 0x28 == 0x274

The command is 0x274, which pads to 0x300. [That 0x274 is needed to trigger the engine (td_size)](https://github.com/eiln/ane/blob/97ca40aec350c73734581b41c411e8af1681649f/ane/src/ane_tm.c#L102-L104).


## L2 Cache

>It operates out of RAM or its 4MB L2 cache. The L2 "cache" appears to be manually managed, and only applies to the input and output, not the weights. The weights are usually included in the program, and it's unclear where they are copied to.

MMIO Map of the engine:

	0x26bc04000 - 0x26bc20000 MADD config
	0x26bc20000 - 0x26bc24000 DMA config
	0x26bc24000 - 0x26bc25000 Task Manager
	0x26bc25000 - 0x26bc28000 Task Queues
	0x26bc34000 - 0x26bc44000 Kernel L2
	0x26bd00000 - 0x26bf00000 Tile L2

Weights/kernel get copied to the kernel L2 range. Total size is misleading because kernel/tiles have separate ranges. Even then, tiles aren't bound to the 0x200000 (2MB) tile L2 size since it starts to utilize intermediate buffers in DRAM (allocated as if source buffers) as swap space when it runs out of L2. L2 isn't manually managed at runtime, but all DMA transfers (involving those to/from L2) are encoded in the command.

Also, [the weights are located at round_up(command_size, 16) for bank aligned access](https://github.com/eiln/ane/blob/97ca40aec350c73734581b41c411e8af1681649f/ane/src/ane_drv.c#L255-L259). The weights are always included in thw hwx because both the command & weights are backed by read-only or immutable DMA channels unlike src/dst buffers.


## Cores

>The 16 cores likely refer to the 16 wide Kernel DMA engine. They claim 11 TOPS total, which would be 687.5 GOPS/core. Perhaps it's a 32x32 MAC running at 335 MHz.

As I said somewhere else: The computation unit of a single-device ANE is its 8 "neural engine" (NE) cores [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf). Some incredible naming btw. Each NE core is a **multiply-add (MADD) unit** composed of 256 **MADD circuits** [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf) and a **memory circuit (accumulator)** [[3]](https://patentimages.storage.googleapis.com/a4/83/a8/ad9d221cb7f8d8/US20190340498A1.pdf). The input value and the corresponding kernel coefficient populated in the register are **multiplied in each of MADD circuits** to generate a processed value [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf), which the **accumulator receives and stores** [[3]](https://patentimages.storage.googleapis.com/a4/83/a8/ad9d221cb7f8d8/US20190340498A1.pdf). NE cores operate in parallel for a total `1 * 8 * 256 = 2048` MADD operations in each processing cycle [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf). Again, there are 8 NE cores. There are performance counters for exactly 8. 16 likely spawned as a marketing term.


## Tiling

>It works with 5D Tensors, you specify the stride for the latter 4. All strides must be a multiple of 0x40 bytes
>* Column (width)    -- aneRegs.Common.InDim.Win / aneRegs.Common.OutDim.Wout
>* Row    (height)   -- aneRegs.Common.InDim.Hin / aneRegs.Common.OutDim.Hout
>* Plane  (channels) -- aneRegs.Common.Cin.Cin / aneRegs.Common.Cout.Cout
>* Depth
>* Group  (batch)    -- aneRegs.Common.GroupConvCfg.NumGroups

[Tensors are 4D NCHW format, Batch (N), Channels (C), Height (H), Width (W)](https://github.com/eiln/anecc/blob/3127e0e350920972a1051bc6e84747268869bc1c/anect/anect/__init__.py#L67-L98). It says this in the plist too (BatchSize, InputChannels, InputHeight, InputWidth). Also, stride isn't specified but derived from NCHW. The plist also has InputRowStride & InputPlaneStride fields. InputRowStride is `round_up((Width * dsize), 64)`. InputPlaneStride is `round_up((InputRowStride * Height), 64)`. Batch/Channels aren't tiled, so total size is [`round_up(Batch * Channels * InputPlaneStride, 0x4000)`](https://github.com/eiln/anecc/blob/3127e0e350920972a1051bc6e84747268869bc1c/anect/anect/__init__.py#L201-L202).


## Base Addresses

>Header -- The base addresses for the DMA engines

Header - Compiler has no access to hardware and thus does not allocate IOMMU space addresses for DMA. No DMA addresses or offsets are encoded in the command, that's done at runtime in the driver. That'd be pretty dangerous otherwise. The header contains fields for task flow management like task id, neural id, fifo mask, context switch (branch) flags, priority parameter, debug flags, etc., see [Task Context Switch for Neural Processor Circuit](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf).

>KernelDMASrc -- 16x wide DMA engine for the weights/bias/nonlinearity

Config for the Kernel DMA channel, which loads segments of the kernel buffer (kernel coefficients) onto the MADDs to perform the, well, MADD with the incoming input. Also handles compression, etc.

>Common -- Specifies the parameters for the convolution

They all do. "Common" just gets repeated before the other 6 fields, hence the addresss is 0x00000.

>L2 -- Use the L2 cache for Source/Result instead of RAM

DRAM.

>It can work with 8 base addresses for the DMA streams per OP
>* 2x Read, both used for things like sum
>* 1x Write
>* 1x T?
>* 4x Kernel, though only the first one seems used

?

BAR (Base Address Register) is a 0x20 (32) hardware queue of iovas (IOMMU virtual address) for DMA channels. It's "base" in the sense of "starting iova" for the rasterizer broadcasting tiled segment workloads, not "base" as in translation tables. More simply, [it's a list of maximum 32 addresses for the DMA channels to use](https://github.com/eiln/ane/blob/97ca40aec350c73734581b41c411e8af1681649f/ane/src/ane_tm.c#L98-L100).

	[0] - command
	[1] - weights
	[2] - intermediate tile 1, if it exists
	[3] - intermediate tile 0, if it exists
	[4] - destination tile 0
	[N] - source tile 0

where dst tile iovas occupy `[4, N)` and src tile iovas occupy `[N, N + src_count]`.


## Hwx File

>hwx file? This is a Mach-O file. We haven't figured out all the details, but the ops are at 0x4000. See `hwx_parse.py`

My guess for the name `hwx` is "hardware executable". The hwx has a macho header, then a buffer section where [buffer counts and sizes are encoded](https://github.com/eiln/anecc/blob/3127e0e350920972a1051bc6e84747268869bc1c/anect/anect/__init__.py#L101). Then, the command is encoded at the next round_up(0x4000) after the buffer section. The buffer section is not constant, and is only <0x4000 for small models. [Here I calculate the command offset using the magic number delimiter trick](https://github.com/eiln/anecc/blob/3127e0e350920972a1051bc6e84747268869bc1c/anect/anect/__init__.py#L186-L190), or 0xf401f800 if you remember. That was a nice wrap up I did not intend.

See the [`anecc`](https://github.com/eiln/anecc) repo for more.
