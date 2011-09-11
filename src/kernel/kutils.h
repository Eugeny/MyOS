#ifndef KUTILS_HPP
#define KUTILS_HPP

#include <util/cpp.h>
#include <tty/Terminal.h>

extern "C" void kprints(char *s);
extern "C" void kprintsp(char *s, int x, int y);

extern "C" char* to_hex(u32int x);
extern "C" char* to_dec(u32int x);

extern "C" void klog_init(Terminal* t);
extern "C" void klog_flush();
extern "C" void klog(char *s);
extern "C" void klogn(char *s);



#define TRACE klogn("Tracing: ");klogn(__FILE__);klogn(":");klog(to_dec( __LINE__));klog_flush();
#define DEBUG(x) klogn("Debug: ");klogn(x);klogn(" at ");klogn(__FILE__);klogn(":");klog(to_dec( __LINE__));klog_flush();

#define PANIC(s) kpanic(__FILE__, __LINE__, s)
extern "C" void kpanic(char* file, u32int line, char* msg);
extern void backtrace();
#endif
