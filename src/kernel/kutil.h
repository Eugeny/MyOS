#ifndef KUTIL_H
#define KUTIL_H

#include "kconfig.h"

#ifdef KCFG_ENABLE_TRACING
    #define KTRACE { ktrace(__FILE__, __LINE__); }
    #define KTRACEMSG(x) { ktrace(__FILE__, __LINE__, x); }
#else
    #define KTRACE ;
    #define KTRACEMSG(x) ;
#endif

void ktrace(const char* file, int line);
void ktrace(const char* file, int line, char* msg);

void sout(const char* str);

#endif
