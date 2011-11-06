#ifndef __SYS_DIRENT
#define __SYS_DIRENT

#define DIR int

#define NAME_MAX 256

struct dirent {
	int d_ino;
	char d_name[NAME_MAX];
} ;


#ifndef KERNEL
DIR *opendir(char*);
struct dirent *readdir(DIR* c);
void rewinddir(DIR* c);
int closedir(DIR* c);
#endif
#endif