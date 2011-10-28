#include "kutils.h"
#include <tty/Terminal.h>
#include <tty/TTYManager.h>
#include <core/Processor.h>
#include <core/TaskManager.h>

Terminal* kterm;

extern "C" void klog_init(Terminal* t) {
    kterm = t;
}

extern "C" void klog_flush() {
    TTYManager::get()->switchActive(0);
    TTYManager::get()->draw();
}

extern "C" void klog(char* s) {
    kterm->write(s);
    kterm->write("\n");
}

extern "C" void klogn(char* s) {
    kterm->write(s);
}

extern "C" void kprints(char *s) {
    unsigned char *vram = (unsigned char *)0xb8000;
    for (;*s;s++,vram+=2) {
        vram[0] = *s;
        vram[1] = 0xf;
    }
}

extern "C" void kprintsp(char *s, int x, int y) {
    unsigned char *vram = (unsigned char *)0xb8000;
    vram += 80*2*y+x*2;
    for (;*s;s++,vram+=2) {
        vram[0] = *s;
        vram[1] = 0xf;
    }
}



#define BS 30
static char buf[BS];

extern "C" char* to_hex(u32int x) {
    char digits[] = "0123456789ABCDEF";
    buf[BS-1] = 0;
    int b = BS - 1;

    if (x == 0)
        buf[b] = '0';

    while (x || b == BS-1) {
        buf[--b] = digits[x%16];
        x /= 16;
    }
    return &(buf[b]);
}

extern "C" char* to_dec(u32int x) {
    buf[BS-1] = 0;
    int b = BS - 1;

    if (x == 0)
        buf[b] = '0';

    while (x || b == BS-1) {
        buf[--b] = '0' + x%10;
        x /= 10;
    }
    return &(buf[b]);
}





extern "C" void kpanic(char* file, u32int line, char* msg) {
    asm volatile("cli");
    klogn("KERNEL PANIC: ");
    klog(msg);
    klogn("at ");
    klogn(file);
    klogn(":");
    klog(to_dec(line));
    backtrace();
    klog_flush();
    for (;;);
}

void procinfo() {
    klogn("Process: ");
    klogn(to_dec(TaskManager::get()->getCurrentThread()->process->pid));
    klogn(" ");
    klog(TaskManager::get()->getCurrentThread()->process->name);
}




#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
  {
    unsigned long int __val[_SIGSET_NWORDS];
  } __sigset_t;


typedef struct sigaltstack
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;


extern u32int start_text, end_text;
void backtrace() {
    u32int bp, ip, c=0;
    asm volatile ("mov %%ebp, %0" : "=r" (bp));
    while (bp < 0xE0000000 && c < 17) {
        if (!TaskManager::get()->getCurrentThread()->process->addrSpace->getPage(bp, false))
            break;
        if (!TaskManager::get()->getCurrentThread()->process->addrSpace->getPage(bp+4, false))
            break;
        ip = *((u32int*)bp+4);
        if (((u32int)(&start_text) < ip && ip < (u32int)(&end_text)) ||
           (0x500000 < ip && ip < 0x3000000)) {
            klogn(to_hex(bp));
            klogn(" ");
            klog(to_hex(ip));
            klog_flush();
            c++;
        }
        bp += 4;
    }

    procinfo();
    klog_flush();
}


void memdump(void* m) {
    klogn("Memory dump at 0x");
    klog(to_hex((u32int)m));
    for (int i = 0; i < 128; i++) {
        klogn(to_hex(0x1000+*(u8int*)(m+i)));
        klogn(" ");
    }
    klog("");
    klog_flush();
}

static    u32int* p;
static    int c=0;
void stacktrace() {
    asm volatile ("mov %%esp, %0" : "=r" (p));

    while ((u32int)p < 0xE0000000 && c <12) {
            klogn(to_hex(*p));
            klogn(" ");
            klog(to_hex((u32int)p));
            c++;
        p++;
    }

    procinfo();
    klog_flush();
}
