#ifndef UTIL_CPP_H
#define UTIL_CPP_H

#define true 1
#define false 0
#define NULL 0


#define PACKED __attribute__((packed))
#define ALIGN(x) __attribute__ ((aligned (x)))

#define restrict __restrict__
#define _CFE_
#define __nonnull

typedef unsigned long  u64int;
typedef          long  s64int;
typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef signed   char  s8int;
typedef u32int         size_t;
typedef s64int         off64_t;

void initialiseConstructors();

void sleep(u32int ms);
int   strlen(char *s);
char* strclone(char *s);
bool  strstarts(char *s, char *p);
bool  strcmp(char *s, char *p);
void *memset(void *s, char d, int l);
void *memcpy(void *dest, const void *src, int n);

void outb(u16int port, u8int);
u8int inb(u16int port);
u16int inw(u16int port);
u32int inl(u16int port);
void insl(u16int port, u32int buf, int count);
#endif
