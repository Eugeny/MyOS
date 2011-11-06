#define DEFN_SYSCALL0(fn, num) \
int fn() \
{ \
  int a; \
  asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
  return a; \
}

#define DEFN_SYSCALL1(fn, num, P1) \
extern int fn(P1 p1) \
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

//#include <errno.h>
//#undef errno
//int errno;


DEFN_SYSCALL1(kprint, 0, char*);

DEFN_SYSCALL0(__exit, 1);
DEFN_SYSCALL0(_exit, 1);
DEFN_SYSCALL0(fork, 2);
DEFN_SYSCALL3(read, 3, void*, unsigned int, unsigned int);
DEFN_SYSCALL3(write, 4, int, void*, unsigned int);
DEFN_SYSCALL3(open, 5, char*, int, int);
DEFN_SYSCALL3(open64, 5, char*, int, int);
DEFN_SYSCALL1(close, 6, int);
DEFN_SYSCALL1(waitpid, 7, int);
DEFN_SYSCALL1(isatty, 8, char*);
DEFN_SYSCALL2(link, 9, char*, char*);
DEFN_SYSCALL1(unlink, 10, char*);

DEFN_SYSCALL2(stat, 18, int, void*);
DEFN_SYSCALL2(lstat, 18, int, void*);
DEFN_SYSCALL2(lstat64, 18, int, void*);
DEFN_SYSCALL2(fstat, 28, int, void*);
DEFN_SYSCALL2(fstat64, 28, int, void*);
DEFN_SYSCALL1(sbrk, 45, unsigned int);


// TODO
DEFN_SYSCALL2(lseek, 10, int, unsigned int);
DEFN_SYSCALL0(getpid, 13);
DEFN_SYSCALL0(getppid, 13);
DEFN_SYSCALL0(sysconf, 13);
DEFN_SYSCALL0(times, 13);
DEFN_SYSCALL3(pipe, 14, char*, unsigned int, int);
DEFN_SYSCALL3(ioctl, 54, unsigned int, unsigned int, int);
DEFN_SYSCALL3(fcntl, 55, unsigned int, unsigned int, int);


typedef void(*thread_entry_point)(void*);
DEFN_SYSCALL2(newThread, 99, thread_entry_point, void*);
DEFN_SYSCALL4(exec, 100, char*, char*, int, void*);

DEFN_SYSCALL1(opendir, 101, char*);
DEFN_SYSCALL1(readdir, 102, int*);
DEFN_SYSCALL1(closedir, 103, int*);




#define uid_t int
#define gid_t int
#define pid_t int
#define sigset_t int


uid_t getuid() { return 0; }
uid_t getgid() { return 0; }
uid_t geteuid() { return 0; }
uid_t getegid() { return 0; }
int setpgid(pid_t pid, pid_t pgid){ return 0; }
pid_t getpgid(pid_t pid){ return 0; }
pid_t getpgrp(void){ return 0; }            /* BSD version */
int setpgrp(void){ return 0; }
int umask(int x) {return 0;}
int getrlimit(int resource, struct rlimit *rlim) {return 0;}
int setrlimit(int resource, const struct rlimit *rlim) {return 0;}

int kill(int pid, int sig) { return 0; }
int killpg(int pid, int sig) { return 0; }

char* strsignal(int sg) { return "SIG"; }

int WCOREDUMP() {}

int wait3(int *s, int o, void* st) {return 0;}

char* getcwd(char* b) { strcpy(b, "/"); return b; }
int chdir(char *d) { return 0; }

       int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact) { return 0; }

       struct passwd *getpwnam(const char *name) { return 0; }

       int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) { return 0; }
       int getgroups(int size, gid_t list[]) { return 0; }

int dup2(int a, int b) { return 0; }

       pid_t tcgetpgrp(int fd) { return 0; }

       int tcsetpgrp(int fd, pid_t pgrp) { return 0; }

