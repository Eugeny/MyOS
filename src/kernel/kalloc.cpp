#include "types.h"
#include "kalloc.h"
#include "kutils.h"
#include "paging.h"

extern u32int end;
static u32int free_space_start = (u32int)&end;



#include "kalloc.heap.cpp"
#define KHEAP_BASE       0xC0000000
#define KHEAP_SIZE       0x1000000 //TODO
#define KHEAP_INDEX_SIZE 0x200000  //TODO
static u8int heap_ready = 0;
heap_t kheap;
 
u32int kmalloc_dumb (u32int sz, u8int align, u32int *phys);
u32int kmalloc_kheap(u32int sz, u8int align, u32int *phys);


// Call the allocator
u32int kmalloc_int(u32int sz, u8int align, u32int *phys) {
    if (!heap_ready)
        return kmalloc_dumb(sz, align, phys);
    else
        return kmalloc_kheap(sz, align, phys);
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

u32int kmalloc_kheap(u32int sz, u8int align, u32int *phys) {
TRACE
    u32int addr = (u32int)heap_alloc(&kheap, align, sz);
    if (phys != 0) {
        page_t *page = paging_get_page((u32int)addr, 0, kernel_directory);
        *phys = page->frame*0x1000 + (((u32int)addr)&0xFFF);
    }
    return addr;
}


extern void kfree(u32int addr) {
    if (heap_ready)
        heap_free(&kheap, addr);
}


extern u32int kalloc_get_boundary() {
    return free_space_start;
}


void kheap_init() {
    kheap.base = KHEAP_BASE;
    kheap.size = KHEAP_SIZE;
    kheap.supervisor = 1;
    kheap.readonly = 0;
    heap_init(&kheap, KHEAP_INDEX_SIZE);
}

void kheap_enable() {    
    heap_ready = 1;
}

void kheap_map_pages() {
    int i = 0;
    for (i = kheap.base; i < kheap.base + kheap.size; i += 0x1000)
        paging_get_page(i, 1, kernel_directory);
}       

void kheap_alloc_pages() {
    int i = 0;
    for (i = kheap.base; i < kheap.base + kheap.size; i += 0x1000)
        paging_alloc_frame(paging_get_page(i, 0, kernel_directory), 0, 0);
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

