#include "kutils.h"
#include "terminal.h"

Terminal kterm;

extern "C" void klog_init() {
//    kterm.direct = 1;
    kterm.reset();
}

extern "C" void klog_flush() {
//    kterm.direct = 1;
    if (kterm.dirty)
        kterm.draw();
}

extern "C" void klog(char* s) {
    kterm.write(s);
    kterm.write("\n");
}

extern "C" void klogn(char* s) {
    kterm.write(s);
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
        buf[b++] = '0';
    
    while (x) {
        buf[--b] = digits[x%16];
        x /= 16;
    }
    return &(buf[b]);
}

extern "C" char* to_dec(u32int x) {
    buf[BS-1] = 0;
    int b = BS - 1;
    
    if (x == 0)
        buf[b++] = '0';
    
    while (x) {
        buf[--b] = '0' + x%10;
        x /= 10;
    }
    return &(buf[b]);
}





extern "C" u16int kpanic(char* file, u32int line, char* msg) {
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

extern "C" u32int read_eip();
void backtrace() {
    u32int ebp, eip;
    eip = read_eip();
    asm volatile ("mov %%ebp, %0" : "=r" (ebp));
   
    while (ebp) {
        klogn(to_hex(eip));
        klogn(" ");
        klog(to_hex(ebp));
        eip = *((u32int*)ebp + 4);
        ebp = *((u32int*)ebp);
    }
    
    klog_flush();
}
