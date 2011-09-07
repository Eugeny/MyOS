#ifndef IDT_H
#define IDT_H

struct idt_entry_struct
{
    u16int base_lo;    // The lower 16 bits of the address to jump to when this interrupt fires.
    u16int sel;     // Kernel segment selector.
    u8int always0;    // This must always be zero.
    u8int flags;    // More flags. See documentation.
    u16int base_hi;    // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct
{
    u16int limit;
    u32int base;    // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;



extern void idt_flush(u32int);
extern void idt_init();
extern void idt_set_gate(u8int,u32int,u16int,u8int);


extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10 ();
extern void isr11 ();
extern void isr12 ();
extern void isr13 ();
extern void isr14 ();
extern void isr15 ();
extern void isr16 ();
extern void isr17 ();
extern void isr18 ();
extern void isr19 ();
extern void isr20 ();
extern void isr21 ();
extern void isr22 ();
extern void isr23 ();
extern void isr24 ();
extern void isr25 ();
extern void isr26 ();
extern void isr27 ();
extern void isr28 ();
extern void isr29 ();
extern void isr30 ();
extern void isr31 ();

extern void isr32 ();
extern void isr33 ();
extern void isr34 ();
extern void isr35 ();
extern void isr36 ();
extern void isr37 ();
extern void isr38 ();
extern void isr39 ();
extern void isr40 ();
extern void isr41 ();
extern void isr42 ();
extern void isr43 ();
extern void isr44 ();
extern void isr45 ();
extern void isr46 ();
extern void isr47 ();

#endif
