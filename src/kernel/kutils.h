#ifndef KUTILS_HPP
#define KUTILS_HPP

#include "types.h"

extern "C" void kprints(char *s);
extern "C" void kprintsp(char *s, int x, int y);

extern "C" char* to_hex(u32int x);
extern "C" char* to_dec(u32int x);

extern "C" void klog_init();
extern "C" void klog_flush();
extern "C" void klog(char *s);
extern "C" void klogn(char *s);
extern "C" void *memset(void *s, char d, int l);
extern "C" void *memcpy(void *dest, const void *src, int n);
extern "C" void outb(u16int port, u8int);
extern "C" u8int inb(u16int port);
extern "C" u16int inw(u16int port);



#define TRACE klogn("Tracing: ");klogn(__FILE__);klogn(":");klog(to_dec( __LINE__));klog_flush();
#define DEBUG(x) klogn("Debug: ");klogn(x);klogn(" at ");klogn(__FILE__);klogn(":");klog(to_dec( __LINE__));klog_flush();

#define PANIC(s) kpanic(__FILE__, __LINE__, s)
extern "C" u16int kpanic(char* file, u32int line, char* msg);

#endif
