#define DEFN_SYSCALL0(fn, num) \
int fn() \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
  return a; \
}

#define DEFN_SYSCALL1(fn, num, P1) \
int fn(P1 p1) \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num), "b" ((int)p1)); \
  return a; \
}

#define DEFN_SYSCALL2(fn, num, P1, P2) \
int fn(P1 p1, P2 p2) \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
  return a; \
}

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
int fn(P1 p1, P2 p2, P3 p3) \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d"((int)p3)); \
  return a; \
}

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
int fn(P1 p1, P2 p2, P3 p3, P4 p4) \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4)); \
  return a; \
}

#define DEFN_SYSCALL5(fn, num) \
int fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4), "D" ((int)p5)); \
  return a; \
}

#include <errno.h>
#undef errno
int errno;


DEFN_SYSCALL1(kprint, 0, char*);
DEFN_SYSCALL0(fork, 1);

typedef void(*thread_entry_point)(void*);
DEFN_SYSCALL2(newThread, 2, thread_entry_point, void*);

DEFN_SYSCALL0(_exit, 3);
DEFN_SYSCALL2(write, 4, void*, unsigned int);
DEFN_SYSCALL2(read, 5, void*, unsigned int);
DEFN_SYSCALL1(sbrk, 6, unsigned int);
DEFN_SYSCALL1(close, 7, int);
DEFN_SYSCALL1(fstat, 8, char*);

DEFN_SYSCALL1(isatty, 9, char*);

DEFN_SYSCALL3(open, 11, char*, unsigned int, int);

// TODO
DEFN_SYSCALL2(lseek, 10, int, unsigned int);
DEFN_SYSCALL1(kill, 12, int);
DEFN_SYSCALL0(getpid, 13);
DEFN_SYSCALL0(getppid, 13);
DEFN_SYSCALL0(sysconf, 13);
DEFN_SYSCALL0(times, 13);
DEFN_SYSCALL3(pipe, 14, char*, unsigned int, int);
DEFN_SYSCALL3(fcntl, 15, char*, unsigned int, int);
