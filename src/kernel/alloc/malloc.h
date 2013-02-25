#ifndef ALLOC_MALLOC_H
#define ALLOC_MALLOC_H

#include "lang/lang.h"


struct kheap_info_t {
    uint32_t total_bytes, used_bytes;
};


extern void* kmalloc(int size);
extern void* kvalloc(int size);
extern void  kfree(void* ptr);

extern kheap_info_t kmallinfo();

#endif