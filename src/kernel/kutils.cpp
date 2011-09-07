#include "kutils.h"

void kprints(char *s) {
    unsigned char *vram = (unsigned char *)0xb8000;
    for (;*s;s++,vram+=2) {
        vram[0] = *s;
        vram[1] = 0xf;
    }
}

void *memset(void *s, char d, int l) {
    for (int i = 0; i < l; i++)
        *(char*)((int)s+i) = d;
    return s;
}

void *memcpy(void *dest, const void *src, int n) {
    int i = 0;
    for (int i = 0; i < n; i++)
        *(char*)((int)dest+i) = *(char*)((int)src+i);
    return dest;
}

void outb(u16int port, u8int value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port)
{
   u8int ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

u16int inw(u16int port)
{
   u16int ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
