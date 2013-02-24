#ifndef INTERRUPTS_INTERRUPTS_H
#define INTERRUPTS_INTERRUPTS_H

#include <lang/lang.h>
#include <lang/Singleton.h>


typedef struct registers
{
    uint32_t tebp, ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
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
