#include <memory/FrameAlloc.h>
#include <alloc/malloc.h>
#include <string.h>
#include <kutil.h>


#define BS_IDX(a) (a / 64)
#define BS_OFF(a) (a % 64)
#define BIT(a) ((uint64_t)1 << a)


void FrameAlloc::init(uint64_t total) {
    totalFrames = total;
    framesBitmap = (uint64_t*)kmalloc(total / 64);
    for (uint64_t i = 0; i < totalFrames / 64; i++)
        framesBitmap[i] = 0;
    usedFrames = 0;
}

void FrameAlloc::markAllocated(uint64_t frame) {
    uint64_t idx = BS_IDX(frame);
    uint8_t  off = BS_OFF(frame);
    //klog('t', "Marking frame %lx", frame);
    if (!(framesBitmap[idx] & BIT(off)))
        usedFrames++;
    framesBitmap[idx] |= BIT(off);
}

uint64_t FrameAlloc::allocate() {
    for (uint64_t i = 0; i < BS_IDX(totalFrames); i++)
        if (framesBitmap[i] != 0xFFFFFFFFFFFFFFFF)
            for (uint64_t j = 0; j < 64; j++)
                if (!(framesBitmap[i] & BIT(j))) {
                    uint64_t frame = i * 64 + j;
                    //klog('t', "Allocated frame %lx", frame);
                    return frame;
                }
    return (uint64_t)(-1);
}

void FrameAlloc::release(uint64_t frame) {
    uint64_t idx = BS_IDX(frame);
    uint8_t  off = BS_OFF(frame);
    //klog('t', "Releasing frame %lx", frame);
    if (framesBitmap[idx] & BIT(off))
        usedFrames--;
    framesBitmap[idx] &= ~(BIT(off));
}

uint64_t FrameAlloc::getTotal() {
    return totalFrames;
}

uint64_t FrameAlloc::getAllocated() {
    return usedFrames;
}
