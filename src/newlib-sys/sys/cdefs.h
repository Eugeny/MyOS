#ifndef SYS_CDEFS__
#define SYS_CDEFS__

#include <unistd.h>


#define strchrnul strchr
#define SSIZE_MAX 9000

uid_t getuid();
uid_t getgid();
uid_t geteuid();
uid_t getegid();

int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);

pid_t getpgrp(void);            /* BSD version */

int setpgrp(void);

#define err(i,f) printf(f)



int umask(int x);

#define rlim_t int

struct rlimit {
	rlim_t rlim_max, rlim_cur;
};

int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);
#define RLIM_INFINITY 9000


int kill(int pid, int sig);
int killpg(int pid, int sig);

char* strsignal(int sg);

int WCOREDUMP();

#define _PATH_DEVNULL "/dev/null"
#define _PATH_TTY "/dev/"

int wait3(int *s, int o, void* st);

#endif