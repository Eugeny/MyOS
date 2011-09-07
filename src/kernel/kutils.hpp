#ifndef KUTILS_HPP
#define KUTILS_HPP

#include "types.h"

extern "C" void kprints(char *s);
extern "C" void klog(char *s);
extern "C" void klogn(char *s);
extern "C" void *memset(void *s, char d, int l);
extern "C" void *memcpy(void *dest, const void *src, int n);
extern "C" void outb(u16int port, u8int);
extern "C" u8int inb(u16int port);
extern "C" u16int inw(u16int port);
#endif
