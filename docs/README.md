
# WIP

# Introduction

The ANE is fundamentally dumb.

Consider Google's TPU is a custom matrix multiplication ASIC built for neural network inference [[1]](https://arxiv.org/pdf/1704.04760.pdf?mod=article_inline). The ANE, designed with [convolutional neural networks (CNNs)](https://en.wikipedia.org/wiki/Convolutional_neural_network?&useskin=vector) in mind [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf), is akin to a fixed-function ASIC for [2D/image convolutions](https://en.wikipedia.org/wiki/Kernel_(image_processing)?&useskin=vector). Apple took the time to define a CNN in their patent, so let's hear it:

> CNN is a class of machine learning techniques that primarily uses convolution between input data and kernel data, which can be decomposed into multiplication and accumulation operations [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf).

Not wrong, but patent speak isn't really intended for human consumption. [Songho's blog](http://www.songho.ca/dsp/convolution/convolution.html) goes into the math of the convolution very well, but you really just need to know that it's the **sum of the element-wise product of *a* and *B*, stored for each section of *A* "sliding-windowed" by *B***. A is called input, B is called kernel. Or, even simpler, many multiply-adds (MADD) arranged in a fancy way. Multiplication and accumulation operations indeed.

Now, hardware: The computation unit of a single-device ANE is its 8 "neural engine" (NE) cores [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf). Some incredible naming btw. Each NE core is a **multiply-add (MADD) unit** composed of 256 **MADD circuits** [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf) and a **memory circuit (accumulator)** [[3]](https://patentimages.storage.googleapis.com/a4/83/a8/ad9d221cb7f8d8/US20190340498A1.pdf). The input value and the corresponding kernel coefficient populated in the register are **multiplied in each of MADD circuits** to generate a processed value [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf), which the **accumulator receives and stores** [[3]](https://patentimages.storage.googleapis.com/a4/83/a8/ad9d221cb7f8d8/US20190340498A1.pdf). NE cores operate in parallel for a total `1 * 8 * 256 = 2048` MADD operations in each processing cycle [[2]](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf).

I call the ANE "stupid" because it's bound to a single computation: **batched multiply-adds**. Specifically, its "microcode" is a series of u32 [hardware register values](https://en.wikipedia.org/w/index.php?title=Register-transfer_level&useskin=vector) that change MADD flow logic in ways that emulate various convolution parameters (dimensions, stride, padding, kernel coordinate, repeat count, there's 0x24000 or 147456 of these), not an instruction set like GPUs have. Thus, it cannot execute arbitrary commands, and all passes are encoded and optimized at compile time. You will face many restrictions on model construction compared to shaders. It's more DSP than GPU unlike its name implies; the main author of these patents actually worked on DSPs before doing ANE stuff.

With specialization comes efficiency, and the One Job it has it does pretty well (power, speed). Understand its capabilities and you might get good use out of it.


# Bib

[1] [In-Datacenter Performance Analysis of a Tensor Processing Unit](https://arxiv.org/pdf/1704.04760.pdf?mod=article_inline)

[2] [Task Context Switch for Neural Processor Circuit](https://patentimages.storage.googleapis.com/09/94/b0/33e4247e137a73/US20220237438A1.pdf)

[3] [Dynamically Shaping and Segmenting Work Units for Processing in Neural Network Processor](https://patentimages.storage.googleapis.com/a4/83/a8/ad9d221cb7f8d8/US20190340498A1.pdf)
