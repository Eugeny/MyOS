#include <syscall/Syscalls.h>

DEFN_SYSCALL1(kprint, 0, char*);
DEFN_SYSCALL0(fork, 1);
DEFN_SYSCALL2(newThread, 2, thread_entry_point, void*);
DEFN_SYSCALL0(die, 3);
DEFN_SYSCALL2(write, 4, void*, u32int);
