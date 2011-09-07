#include "kutils.h"
#include "types.h"

typedef struct registers
{
    u32int ds;                  // Data segment selector
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    u32int int_no, err_code;    // Interrupt number and error code (if applicable)
    u32int eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;


char s[] = "  xx  xxxxxx";



void interrupt_handler(registers_t regs) {
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

//static int  INTC=10;
//    if (INTC-- == 0)
//        for(;;);
}

int tick=0;

void irq_handler(registers_t regs) {
    klogn("IRQ ");

    int n = regs.int_no;
        
    if (n==0) n = tick++;
    s[2] = (char)((int)'0' + n/100);
    s[3] = (char)((int)'0' + n/10%10);
    s[4] = (char)((int)'0' + n%10);

    s[6] = 0;
    klog(s);
    
    if (regs.int_no >= 8)
    {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}


extern void isr_handler(registers_t regs)
{
    if (regs.int_no < 32) {
        interrupt_handler(regs);    
    } else {
        regs.int_no -= 32;
        irq_handler(regs);
    }
}
