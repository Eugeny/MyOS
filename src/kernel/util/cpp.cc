#include <memory/Heap.h>
#include <kutils.h>


// Standard library

void sleep(u32int ms) {
    for (int i = 0; i < 10*ms; i++);
}

int strlen(char *s) {
    int r = 0;
    while (*(s++)) r++;
    return r;
}

char* strdup(char* s) {
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
    for (volatile int i = 0; i < n; i++) {
        //*(u8int*)((u32int)dest+i) = *(u8int*)((u32int)src+i);
        u8int* a = (u8int*)(src+i);
        u8int* b = (u8int*)(dest+i);
        *b = *a;
    }
    u8int* a = (u8int*)(src+4);
    u8int* b = (u8int*)(dest+4);
    *b = *a;

    a = (u8int*)(src+8);
    b = (u8int*)(dest+8);
    *b = *a;
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

u32int inl(u16int port) {
   u32int ret;
   asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

void insl(u16int port, u32int buf, int count) {
    for (int i = 0; i < count; i++)
        *((u32int*)buf+i*4) = inl(port);
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
