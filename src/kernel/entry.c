#include "kutils.h"
#include "gdt.h"
#include "idt.h"

extern void kmain (void* mbd, unsigned int magic)
{
    if (magic != 0x2BADB002)
    {
    }
    
    klog_init();
    
    gdt_init();
    idt_init();

    init_timer(100);
              
    klog("Working...");
    
    char s[] = "Test  ";
    int c = 0,i=0;
    while (1) {
        s[5] = (char)((int)'0' + c++%10);
        klog(s);
        for (i=0;i<300000000;i++);
        asm ("sti");
    }
///    putchar((int)"o");
    for(;;);
}
