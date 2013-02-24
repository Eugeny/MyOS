#ifndef KUTIL_H
#define KUTIL_H

#include "kconfig.h"
#include <stdio.h>


#ifdef KCFG_ENABLE_TRACING
    #define KTRACE { ktrace(__FILE__, __LINE__); }
    #define KTRACEMSG(x) { ktrace(__FILE__, __LINE__, x); }
    #define KTRACEMEM { ktracemem(__FILE__, __LINE__); }
#else
    #define KTRACE ;
    #define KTRACEMSG(x) ;
    #define KTRACEMEM ;
#endif

void ktrace(const char* file, int line);
void ktrace(const char* file, int line, const char* msg);
void ktracemem(const char* file, int line);

void microtrace();
void sout(const char* str);

#endif
