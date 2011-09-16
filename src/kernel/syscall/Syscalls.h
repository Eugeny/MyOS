#ifndef SYSCALL_SYSCALLS_H
#define SYSCALL_SYSCALLS_H

#include <core/Thread.h>

#define DECL_SYSCALL0(fn) extern "C" int fn();
#define DECL_SYSCALL1(fn,p1) extern "C" int fn(p1);
#define DECL_SYSCALL2(fn,p1,p2) extern "C" int fn(p1,p2);
#define DECL_SYSCALL3(fn,p1,p2,p3) extern "C" int fn(p1,p2,p3);
#define DECL_SYSCALL4(fn,p1,p2,p3,p4) extern "C" int fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn,p1,p2,p3,p4,p5) extern "C" int fn(p1,p2,p3,p4,p5);



DECL_SYSCALL1(kprint, char*);
DECL_SYSCALL0(fork);
DECL_SYSCALL2(newThread, thread_entry_point, void*);
DECL_SYSCALL0(die);
DECL_SYSCALL2(write, void*, u32int);
DECL_SYSCALL1(sbrk, unsigned int);
#endif
