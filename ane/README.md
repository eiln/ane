


# Legit Bugs

### 1. Engine Interrupt

It *seems* like the engine is sending an interrupt
when it's done. Any interactions with this interrupt 
isn't under CPU domain under macos because 
handler is embedded in the ASC firmware.
I can't trace it, so I don't know how to do anything
with it, notably ack it so it doesn't bang the 
irq handler 100K times. It's always *slightly* 
later than the readl poll, "later" as in my handler is
done by then, so I can't really find a reason to use it. 

For whatever reason, this interrupt goes
to the dart AIC line, so the poor dart irq handler 
is yelled at to do shit it's never even heard of. 
I had to swap AIC numbers in the device tree to even work with it.
Sadly this means the dart irq (only for the ane) has to be disabled. 
If I don't do it, it'll end up disabling itself after
the inevitable irq exahust. 
The dart irq handler handles translation faults, 
so those messages won't be received. 

`cat -n ane.c | grep sigh` to find. 



### ~~2. Dirty Iovas~~

~~Somewhere in the DMA sequence complains (panics) when trying to reuse (what I believe is fully unmapped) old iovas. So the iova allocator leaks on purpose. Weirdly, things work when I reset the allocator by unloading & reloading the module completely. This tells me there's some kind of leak, but I've had this problem for quite a while and without much progress.~~

~~Not too long ago I figured out the iommu_unmap() call has to be looped; since the iova was continuous and the dart pte func walks the given range, I thought the unmap call could be done in one go. But apparently not because with the looped unmap function I can force reload to reset the allocator! Hooray!~~

~~Yeah, obviously really, really terrible and merely a cheap hack for when I run out of addresses. Any help more than welcome.~~

Nope. Just figured it out.
Had to invalidate TLBs for dart domain 1 & 2.



### 3. Power Domains

I don't know why they're not called on probe. 
The always-on flag is set to prevent MMIO faults.
Hasn't been fixed yet, but should not be too hard.




# Could be Better 

1. Handle fifo hack in a less uglier manner.

2. Tile types could be a bitmask instead of =='s across 
the u32's taken from user struct. Added it not too long ago
for the chained stuff.

3. T600x ane0 support. Let's get T8103 working first, but 
I'm hopeful these can work with just mmio base offsets + 
power domain tweaks.

4. Multi-device support. There's no foundation for ane2 yet. 

5. Hard 16K page assumption. Won't probe otherwise.
This is because tiles, the engine's unit of operation, are 16K.
I guess it could work on 4K with a special mmap call to
distribute 16K-continuous pages, but, eh.


