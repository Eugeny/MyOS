#ifndef KUTILS_C 
#define KUTILS_C

#include "types.h"

extern void kprints(char *s);
extern void *memset(void *s, char d, int l);
extern void *memcpy(void *dest, const void *src, int n);
extern void outb(u16int port, u8int);
extern u8int inb(u16int port);
extern u16int inw(u16int port);
#endif
