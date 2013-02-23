#include "kutil.h"
#include <stdio.h>
#include "alloc/malloc.h"


int head[10240];

extern "C" void* __wrap_malloc(int c) {
    void* r = kmalloc(c); // (void*)head;

    char buf[1024];
    sprintf(buf, "malloc(%i) = %i", c, r);
    //KTRACEMSG(buf);
    
    return r;
}

extern "C" void* __wrap_sbrk(int c) {
    void* r = 0;

    char buf[1024];
    sprintf(buf, "sbrk(%i) = %i", c, r);
    //KTRACEMSG(buf);
    
    return r;
}



void* operator new(size_t s) { return __wrap_malloc(s); }
void operator delete(void* p) { }
void* operator new[](size_t s) { return __wrap_malloc(s); }
void operator delete[](void* p) { }

