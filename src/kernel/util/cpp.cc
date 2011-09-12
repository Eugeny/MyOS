#include <memory/Heap.h>
#include <kutils.h>


// Standard library

int strlen(char *s) {
    int r = 0;
    while (*(s++)) r++;
    return r;
}

char* strclone(char* s) {
    char* r = (char*)kmalloc(strlen(s)+1);
    memcpy(r, s, strlen(s)+1);
    return r;
}

bool strstarts(char *s, char *p) {
    for (int i = 0; i < strlen(p); i++)
        if (s[i] != p[i])
            return false;
    return true;
}

bool strcmp(char *s, char *p) {
    if (strlen(s) != strlen(p))
        return false;
    for (int i = 0; i < strlen(p); i++)
        if (s[i] != p[i])
            return false;
    return true;
}

void *memset(void *s, char d, int l) {
    for (int i = 0; i < l; i++)
        *(char*)((int)s+i) = d;
    return s;
}

void *memcpy(void *dest, const void *src, int n) {
    for (int i = 0; i < n; i++)
        *(char*)((int)dest+i) = *(char*)((int)src+i);
    return dest;
}

void outb(u16int port, u8int value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port) {
   u8int ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

u16int inw(u16int port) {
   u16int ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}




// C++ Language Support
void *__dso_handle;


// Defined in the linker.
extern "C" u32int start_ctors;
extern "C" u32int end_ctors;

void initialiseConstructors()
{
    u32int *iterator = reinterpret_cast<u32int*>(&start_ctors);
    while (iterator < reinterpret_cast<u32int*>(&end_ctors))
        (reinterpret_cast<void (*)(void)>(*iterator++))();
}

extern "C" void __cxa_atexit(void (*f)(void *), void *p, void *d) { }
extern "C" void __cxa_pure_virtual() {}
extern "C" int __cxa_guard_acquire() { return 1; }
extern "C" void __cxa_guard_release() { }

void *operator new (size_t size) {
    return kmalloc(size);
}

void *operator new[] (size_t size) throw() {
    return kmalloc(size);
}

void *operator new (size_t size, void* memory) {
    return memory;
}

void *operator new[] (size_t size, void* memory) {
    return memory;
}

void operator delete (void * p) {
    kfree(p);
}

void operator delete[] (void * p) {
    kfree(p);
}

void operator delete (void *p, void *q) {
    PANIC("Operator delete -implement");
}

void operator delete[] (void *p, void *q) {
    PANIC("Operator delete[] -implement");
}
