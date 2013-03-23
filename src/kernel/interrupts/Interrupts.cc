#include <interrupts/Interrupts.h>
#include <kutil.h>
#include <string.h>
#include <hardware/io.h>


#define IDT_SIZE 256

static interrupt_handler interrupt_handlers[IDT_SIZE];


void Interrupts::removeAllHandlers() {
    klog('d', "Removing all interrupt handlers");
    memset(interrupt_handlers, 0, sizeof(interrupt_handler)*IDT_SIZE);
}

void Interrupts::setHandler(int n, interrupt_handler h) {
    klog('d', "Setting interrupt handler #%i", n);
    interrupt_handlers[n] = h;
}

void Interrupts::removeHandler(int n) {
    klog('d', "Removing interrupt handler #%i", n);
    interrupt_handlers[n] = NULL;
}


static uint64_t interrupt_counter = 0;

static void default_interrupt_handler(isrq_registers_t* regs) {
    if (regs->int_no == 7) {
        // Mute INT7
        return;
    }

    klog('w', "Uncaught interrupt #%i, ec %x, RIP 0x%lx, counter %u", 
        regs->int_no, regs->err_code, regs->rip, interrupt_counter);
}

static void default_irq_handler(isrq_registers_t* regs) {
    klog('w', "Uncaught IRQ #%i, counter %u", regs->int_no, interrupt_counter);
}



void INTERRUPT_MUTE(isrq_registers_t* regs) {}

extern "C" void isr_handler(isrq_registers_t* regs) {
    //__output("INT", 10);
    //__outputhex(regs->int_no, 20);
    //__outputhex(regs->rip, 40);
    //__outputhex(regs->rsp, 60);
    interrupt_counter++;

    regs->int_no %= 256;
    bool irq = (regs->int_no >= 32 && regs->int_no <= 47);
    int ino = regs->int_no;
    if (irq)
        regs->int_no -= 32;

    if (irq) {
        // ACK IRQ
        if (regs->int_no >= 8)
            outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }

    if (interrupt_handlers[ino] != 0) {
        interrupt_handlers[ino](regs);
    } else {
        if (!irq) {
            default_interrupt_handler(regs);
        } else {
            default_irq_handler(regs);
        }
    }
}
