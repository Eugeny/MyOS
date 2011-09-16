#ifndef INTERRUPTS_INTERRUPTS_H
#define INTERRUPTS_INTERRUPTS_H

#include <util/cpp.h>
#include <util/Singleton.h>


typedef struct registers
{
    u32int tebp, ds;                  // Data segment selector
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    u32int int_no, err_code;    // Interrupt number and error code (if applicable)
    u32int eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} isrq_registers_t;


#define IRQ(x)  ((x)+32)

typedef void (*interrupt_handler)(isrq_registers_t*);


class Interrupts : public Singleton<Interrupts> {
public:
    void removeAllHandlers();
    void setHandler(int n, interrupt_handler h);
    void removeHandler(int n);
};

#endif
