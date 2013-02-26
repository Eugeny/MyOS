#ifndef MEMORY_FRAMEALLOC_H
#define MEMORY_FRAMEALLOC_H

#include <lang/lang.h>
#include <lang/Singleton.h>

class FrameAlloc : public Singleton<FrameAlloc> {
public:
    void init(uint64_t total);
    uint64_t allocate();
    void markAllocated(uint64_t frame);
    void release(uint64_t frame);
    uint64_t getTotal();
    uint64_t getAllocated();
private:    
    uint64_t *framesBitmap;
    uint64_t totalFrames;
    uint64_t usedFrames;
};
#endif
