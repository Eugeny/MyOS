#include "kutil.h"
#include <stdio.h>
#include "alloc/malloc.h"


extern "C" void* __wrap_malloc(int c) {
    void* r = kmalloc(c);
    return r;
}

#include <string.h>


extern "C" void __wrap_memset(void *s, int c, size_t n) {
    if (n > 0)
        for (uint64_t i = 0; i < n; i++)
            *((uint8_t*)s + i) = c;
}

extern "C" void* __wrap_memcpy1(void *d, const void *s, size_t n) {
    for (uint64_t i = 0; i < n; i++)
        *((uint8_t*)d + i) = *((uint8_t*)s + i);
    return d;
}





void* operator new(size_t s) { 
    return __wrap_malloc(s); 
}

void operator delete(void* p) { 
    kfree(p);
}

void* operator new[](size_t s) { 
    return __wrap_malloc(s); 
}

void operator delete[](void* p) { 
    kfree(p);
}
