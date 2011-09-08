#include "kutils.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "isr.h"
#include "kalloc.h"
#include "paging.h"


void on_timer(registers_t r) {
    static int tick = 0;
    char s[] = "Timer tick 0000";
    s[11] = '0' + tick/1000%10;
    s[12] = '0' + tick/100%10;
    s[13] = '0' + tick/10%10;
    s[14] = '0' + tick%10;
    tick++;
    kprintsp(s, 60, 0);
}

 
extern "C" void kmain (void* mbd, unsigned int magic)
{
    if (magic != 0x2BADB002)
    {
    }
    
    klog_init();
    
    gdt_init();
    idt_init();

    reset_interrupt_handlers();
    init_timer(1000);
    
    set_interrupt_handler(IRQ(0), on_timer);
     
    klog("Starting paging");
    paging_init();
    
    klogn("malloc ");
    klog(to_hex(kmalloc(400)));
    klogn("malloc ");
    klog(to_hex(kmalloc(4000)));
    klogn("malloc ");
    klog(to_hex(kmalloc(40000)));
     
     
    u32int x = *((u32int*)0xFFFFFFFF);       
    /*
    klog("Working...");
    
    char s[] = "Test  ";
    int c = 0,i=0;
    while (1) {
        s[5] = (char)((int)'0' + c++%10);
        klog(s);
        for (i=0;i<300000000;i++);
    }
///    putchar((int)"o");
    */
    for(;;);
}


