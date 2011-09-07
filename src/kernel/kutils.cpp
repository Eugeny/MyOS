#include "kutils.hpp"
#include "terminal.hpp"

Terminal kterm;

extern "C" void klog_init() {
    kterm.reset();
}

extern "C" void klog(char* s) {
    kterm.write(s);
    kterm.write("\n");
    kterm.draw();
}

extern "C" void klogn(char* s) {
    kterm.write(s);
    kterm.draw();
}

extern "C" void kprints(char *s) {
    unsigned char *vram = (unsigned char *)0xb8000;
    for (;*s;s++,vram+=2) {
        vram[0] = *s;
        vram[1] = 0xf;
    }
}

extern "C" void *memset(void *s, char d, int l) {
    for (int i = 0; i < l; i++)
        *(char*)((int)s+i) = d;
    return s;
}

extern "C" void *memcpy(void *dest, const void *src, int n) {
    for (int i = 0; i < n; i++)
        *(char*)((int)dest+i) = *(char*)((int)src+i);
    return dest;
}

extern "C" void outb(u16int port, u8int value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

extern "C" u8int inb(u16int port)
{
   u8int ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

extern "C" u16int inw(u16int port)
{
   u16int ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
