#ifndef KUTIL_H
#define KUTIL_H

#include <lang/lang.h>
#include <kconfig.h>
#include <stdio.h>


#ifdef KCFG_ENABLE_TRACING
    #define KTRACE { ktrace(__FILE__, __LINE__); }
    #define KTRACEMSG(x) { ktrace(__FILE__, __LINE__, x); }
#else
    #define KTRACE ;
    #define KTRACEMSG(x) ;
#endif

#ifdef KCFG_ENABLE_MEMTRACING
    #define KTRACEMEM { ktracemem(__FILE__, __LINE__); }
#else 
    #define KTRACEMEM ;
#endif

void ktrace(const char* file, int line);
void ktrace(const char* file, int line, const char* msg);
void ktracemem(const char* file, int line);


void __outputhex(uint64_t h, int offset);
void __output(const char* s, int offset);
void microtrace();

void klog_init();
void klog_init_terminal();
void klog(char type, const char* format, ...);
void klog_flush();

void dump_stack(uint64_t, uint64_t);

void seterr(int err);
bool haserr();
int geterr();

//void sout(const char* str);

#endif
