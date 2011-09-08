#include "types.h"


extern u32int end;
static u32int free_space_start = (u32int)&end;



static u8int heap_ready = 0;
u32int kmalloc_dumb(u32int sz, u8int align, u32int *phys);


// Call the allocator
u32int kmalloc_int(u32int sz, u8int align, u32int *phys) {
    if (!heap_ready)
        kmalloc_dumb(sz, align, phys);
}

u32int kmalloc_dumb(u32int sz, u8int align, u32int *phys) {
    u32int addr = free_space_start;
    
    if (align == 1 && (addr & 0xFFF)) {
        addr &= 0xFFFFF000;
        addr += 0x1000;
    }
    if (phys) {
        *phys = addr;
    }
    u32int tmp = addr;
    addr += sz;
    free_space_start = addr;
    return tmp;
}

extern u32int kalloc_get_boundary() {
    return free_space_start;
}





extern u32int kmalloc_a(u32int sz) {
    return kmalloc_int(sz, 1, 0);
}

extern u32int kmalloc_p(u32int sz, u32int *phys) {
    return kmalloc_int(sz, 0, phys);
}

extern u32int kmalloc_ap(u32int sz, u32int *phys) {
    return kmalloc_int(sz, 1, phys);
}

extern u32int kmalloc(u32int sz) {
    return kmalloc_int(sz, 0, 0);
}

extern void kfree(u32int addr) {
}

