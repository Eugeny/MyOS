#include "kutil.h"
#include <stdio.h>
#include "alloc/malloc.h"


bool __heap_ready = false;

extern "C" void* __wrap_malloc(int c) {
    void* r = kmalloc(c);
    __heap_ready = true;

    #ifdef KCFG_ENABLE_TRACING
    #endif
    
    return r;
}

#include <string.h>
// DEFUNCT
extern "C" void __wrap____printf_fp(void* ptr) {
    microtrace();

    if (__heap_ready) {
    #ifdef KCFG_ENABLE_TRACING
    #endif

    //kfree(ptr);
    }
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
