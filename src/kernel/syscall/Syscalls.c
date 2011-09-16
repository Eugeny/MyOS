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


typedef void(*thread_entry_point)(void*);


DEFN_SYSCALL1(kprint, 0, char*);
DEFN_SYSCALL0(fork, 1);
DEFN_SYSCALL2(newThread, 2, thread_entry_point, void*);
DEFN_SYSCALL0(die, 3);
DEFN_SYSCALL2(write, 4, void*, unsigned int);
DEFN_SYSCALL2(read, 5, void*, unsigned int);
DEFN_SYSCALL1(sbrk, 6, unsigned int);
DEFN_SYSCALL1(close, 7, int);
//DEFN_SYSCALL1(fstat, 8, char*);
#include <sys/stat.h>
    int fstat(int file, struct stat *st) {
      st->st_mode = S_IFCHR;
      return 0;
    }

//DEFN_SYSCALL1(isatty, 9, char*);
    int isatty(int file){
       return 1;
    }

DEFN_SYSCALL2(lseek, 10, int, unsigned int);
DEFN_SYSCALL3(open, 11, char*, unsigned int, int);
