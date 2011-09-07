#include "kutils.h"
#include "types.h"

typedef struct registers
{
    u32int ds;                  // Data segment selector
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    u32int int_no, err_code;    // Interrupt number and error code (if applicable)
    u32int eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;


char s1[] = "INTERRUPT\n";
char s[] = "DATA xx xxxxxx\n";

#include "terminal.h"
extern Terminal t;


static int  INTC=10;

extern "C" void isr_handler(registers_t regs)
{
    t.write(s1);
    t.draw();
    s[4] = (char)((int)'0' + regs.int_no/100);
    s[5] = (char)((int)'0' + regs.int_no/10%10);
    s[6] = (char)((int)'0' + regs.int_no%10);

    s[8] = (char)((int)'0' + regs.err_code/100000%10);
    s[9] = (char)((int)'0' + regs.err_code/10000%10);
    s[10] = (char)((int)'0' + regs.err_code/1000%10);
    s[11] = (char)((int)'0' + regs.err_code/100%10);
    s[12] = (char)((int)'0' + regs.err_code/10%10);
    s[13] = (char)((int)'0' + regs.err_code%10);
    t.write(s);
    t.draw();
    
    if (INTC-- == 0)
        for(;;);
}
