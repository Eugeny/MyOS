#ifndef ISR_H
#define ISR_H

#include "types.h"

typedef struct registers
{
    u32int ds;                  // Data segment selector
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    u32int int_no, err_code;    // Interrupt number and error code (if applicable)
    u32int eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

#define IRQ(x)  ((x)+32)
typedef void (*interrupt_handler)(registers_t);
extern void reset_interrupt_handlers();
extern void set_interrupt_handler(int n, interrupt_handler h);
extern void clear_interrupt_handler(int n);


#endif
