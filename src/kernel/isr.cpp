#include "kutils.h"
#include <util/cpp.h>
#include "isr.h"


interrupt_handler interrupt_handlers[256];



void reset_interrupt_handlers() {
    memset(interrupt_handlers, 0, sizeof(interrupt_handler)*256);
}

void set_interrupt_handler(int n, interrupt_handler h) {
    interrupt_handlers[n] = h;
}

void clear_interrupt_handler(int n) {
    interrupt_handlers[n] = 0;
}


void default_interrupt_handler(registers_t regs) {
    char s[] = "  xx  xxxxxx";

    klogn("INT ");
    s[2] = (char)((int)'0' + regs.int_no/100);
    s[3] = (char)((int)'0' + regs.int_no/10%10);
    s[4] = (char)((int)'0' + regs.int_no%10);

    s[6] = (char)((int)'0' + regs.err_code/100000%10);
    s[7] = (char)((int)'0' + regs.err_code/10000%10);
    s[8] = (char)((int)'0' + regs.err_code/1000%10);
    s[9] = (char)((int)'0' + regs.err_code/100%10);
    s[10] = (char)((int)'0' + regs.err_code/10%10);
    s[11] = (char)((int)'0' + regs.err_code%10);
    klog(s);
}

void default_irq_handler(registers_t regs) {
    char s[] = "  xx  xxxxxx";

    klogn("IRQ ");

    int n = regs.int_no;
        
    s[2] = (char)((int)'0' + n/100);
    s[3] = (char)((int)'0' + n/10%10);
    s[4] = (char)((int)'0' + n%10);

    s[6] = 0;
    klog(s);
}


extern "C" void isr_handler(registers_t regs)
{
    int ino = regs.int_no;
    if (regs.int_no >= 32)
        regs.int_no -= 32;

    if (ino >= 32) {
        if (regs.int_no >= 8)
        {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);        
    }
    
    
    if (interrupt_handlers[ino] != 0) {
        interrupt_handlers[ino](regs);
    } else {
        if (ino < 32) {
            default_interrupt_handler(regs);    
        } else {
            default_irq_handler(regs);
        }
    }
}
