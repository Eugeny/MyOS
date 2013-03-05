#ifndef SYSCALL_SYSCALLS_H
#define SYSCALL_SYSCALLS_H

#include <lang/lang.h>

struct syscall_regs_t {
    uint64_t ursp, urip, id;
    uint64_t rbp, rdi, rsi, r15, r14, r13, r12, r10, r9, r8, rdx, fs, gs;
};


class Syscalls {
public:
    static void init();
    static uint64_t error();
};

#endif