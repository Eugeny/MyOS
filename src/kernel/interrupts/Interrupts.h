#ifndef INTERRUPTS_INTERRUPTS_H
#define INTERRUPTS_INTERRUPTS_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <core/MQ.h>


typedef struct registers
{
    uint64_t rbp, rdi, rsi, r15, r14, r13, r12, r11, r10, rdx, rcx, rbx, rax; // Pushed by pusha.
    uint8_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint64_t dunno, rip, rflags, ss, rsp; // Pushed by the processor automatically.
} isrq_registers_t;


#define IRQ(x)  ((x)+32)

typedef void (*interrupt_handler)(isrq_registers_t*);

void INTERRUPT_MUTE(isrq_registers_t* regs);

class Interrupts : public Singleton<Interrupts> {
public:
    void removeAllHandlers();
    void setHandler(int n, interrupt_handler h);
    void removeHandler(int n);
};

#endif
