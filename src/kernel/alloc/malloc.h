#ifndef ALLOC_MALLOC_H
#define ALLOC_MALLOC_H

#include "lang/lang.h"


struct kheap_info_t {
    uint32_t total_bytes, used_bytes;
};


void kalloc_switch_to_main_heap();

void* kmalloc(int size);
void* kvalloc(int size);
void  kfree(void* ptr);
void  kmalloc_trim();

kheap_info_t kmallinfo();

#endif