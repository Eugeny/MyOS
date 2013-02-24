#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <util/cpp.h>
#include <util/Singleton.h>

#define HEAP_HOLE 1

typedef struct heap_item {
    u32int addr;
    u32int size;
    u8int flags;
} heap_item_t;

typedef struct heap {
    heap_item_t  *index;
    u32int        index_length;
    u32int        base;
    u32int        size;
} heap_t;



class Heap : public Singleton<Heap> {
public:
    static void  _selfinit(Heap *h);
    void   init();
    void*  malloc(u32int sz);
    void*  malloc(u32int sz, bool align, u32int *phys);
    void   free(void* addr);

    u32int getFreeSpaceBoundary();
    u32int getUsage();
    void   switchToHeap();
private:
    u32int malloc_dumb(u32int sz, u8int align, u32int *phys);
    u32int malloc_kheap(u32int sz, u8int align, u32int *phys);
    bool heap_ready;
    heap_t kheap;
};


// For pre-malloc initialization
extern void heap_selfinit();

extern void *kmalloc_a(u32int sz);
extern void *kmalloc_p(u32int sz, u32int *phys);
extern void *kmalloc_ap(u32int sz, u32int *phys);
extern "C" void *kmalloc(u32int sz);
extern "C" void  kfree(void* addr);

#endif