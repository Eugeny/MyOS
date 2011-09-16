#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <util/cpp.h>

class Process;

typedef void(*thread_entry_point)(void*);


typedef struct TSS32 {
   unsigned short backlink, __blh;
   unsigned long esp0;
   unsigned short ss0, __ss0h;
   unsigned long esp1;
   unsigned short ss1, __ss1h;
   unsigned long esp2;
   unsigned short ss2, __ss2h;
   unsigned long cr3, eip, eflags;
   unsigned long eax,ecx,edx,ebx,esp,ebp,esi,edi;
   unsigned short es, __esh;
   unsigned short cs, __csh;
   unsigned short ss, __ssh;
   unsigned short ds, __dsh;
   unsigned short fs, __fsh;
   unsigned short gs, __gsh;
   unsigned short ldt, __ldth;
   unsigned short trace, iomapbase;
} tss_t;


class Thread {
public:
    Thread(Process *p);
    ~Thread();
    bool   dead;

    u32int id;
    u32int stackBottom;
    u32int stackSize;
    Process *process;
    tss_t* TSS;
private:
};

#endif
