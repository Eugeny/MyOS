#include "kutil.h"
#include <stdio.h>
#include "alloc/malloc.h"


bool __heap_ready = false;

extern "C" void* __wrap_malloc(int c) {
    void* r = kmalloc(c);
    __heap_ready = true;

    #ifdef KCFG_ENABLE_TRACING
    char buf[1024];
    sprintf(buf, "malloc(%i) = %i", c, r);
    KTRACEMSG(buf);
    #endif
    
    return r;
}

#include <string.h>
// DEFUNCT
extern "C" void __wrap_free(void* ptr) {
    char buffer[102];
    microtrace();

    if (__heap_ready) {
    #ifdef KCFG_ENABLE_TRACING
    sprintf(buffer, "free(%i)\0", 5);
    //strcpy(buffer,"qwe");
    
    for(;;);
    KTRACEMSG(buffer);
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
