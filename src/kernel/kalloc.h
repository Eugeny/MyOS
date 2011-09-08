#ifndef MMU_H
#define MMU_H

#include "types.h"

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
    u8int         supervisor;
    u8int         readonly;
} heap_t;

#define HEAP_HOLE 1


extern u32int kalloc_get_boundary();
extern u32int kmalloc_a(u32int sz);
extern u32int kmalloc_p(u32int sz, u32int *phys);
extern u32int kmalloc_ap(u32int sz, u32int *phys);
extern u32int kmalloc(u32int sz);
extern void   kfree(u32int addr);

extern void kheap_init();
extern void kheap_map_pages();
extern void kheap_alloc_pages();
extern void kheap_enable();
#endif 
