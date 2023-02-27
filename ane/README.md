


# Legit Bugs

### 1. Engine Interrupt

It *seems* like the engine is sending an interrupt when it's done.
Any interactions with this signal isn't under CPU domain under macos
because the handler is embedded in the ASC firmware.
As I can't trace it, I don't know how to do anything with it,
notably ack it so it doesn't bang the irq handler 100K times.
Not only is the signal unreliable with runtime pm enabled,
when it does occur, it's slower than the the native readl poll;
hence I can't really find a reason to use it.

For whatever reason\*\* this interrupt goes
to the dart AIC line, so the poor dart irq handler 
is yelled at to do shit it's never even heard of.
I had to swap AIC numbers in the device tree to even work with it.
Sadly this means the dart irq (only for the ane) has to be disabled.
If I don't do it, it'll end up disabling itself after the inevitable irq exahust.
Consequently, translation faults sent to the dart irq handler won't be received.

`cat -n ane.c | grep sigh` to find. 


\*\*In hindsight this may be due to that
the engine registers are accessed from the CPU and not ASC.
Dart bypass range in the DT includes the iboot-mapped firmware address.




### 2. ~~Dirty Iovas~~

~~Somewhere in the DMA sequence complains (panics) when trying to reuse (what I believe is fully unmapped) old iovas. So the iova allocator leaks on purpose. Weirdly, things work when I reset the allocator by unloading & reloading the module completely. This tells me there's some kind of leak, but I've had this problem for quite a while and without much progress.~~

~~Not too long ago I figured out the iommu_unmap() call has to be looped; since the iova was continuous and the dart pte func walks the given range, I thought the unmap call could be done in one go. But apparently not because with the looped unmap function I can force reload to reset the allocator! Hooray!~~

~~Yeah, obviously really, really terrible and merely a cheap hack for when I run out of addresses. Any help more than welcome.~~

Nope. Just figured it out.
Had to invalidate TLBs for dart domain 1 & 2.



### 3. ~~Power Domains~~

~~I don't know why they're not called on probe. The always-on flag is set to prevent MMIO faults. Hasn't been fixed yet, but should not be too hard.~~

Fixed. 





# Could be Better 

- Handle fifo hack in a less uglier manner

- T6000 support. Let's get T8103 working first, but 
I'm hopeful ane0 can work OOTB with power domain tweaks.

- T6000 ane2 support... Sometime...

- Hard 16K page assumption. Won't probe otherwise.
This is because tiles, the engine's unit of operation, are 16K.
I guess it could work on 4K with a special mmap call to
distribute 16K-continuous pages, but, eh.
