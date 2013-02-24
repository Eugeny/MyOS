#include <interrupts/IDT.h>
#include <hardware/io.h>
#include <string.h>


typedef struct idt_entry_struct {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} PACKED idt_entry_t;


typedef struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} PACKED idt_ptr_t;


static idt_entry_t ALIGN(16) idt_entries[256];
static idt_ptr_t   ALIGN(16) idt_ptr;

extern "C" void _IDT_Flush(uint32_t);


extern "C" void isr0 ();
extern "C" void isr1 ();
extern "C" void isr2 ();
extern "C" void isr3 ();
extern "C" void isr4 ();
extern "C" void isr5 ();
extern "C" void isr6 ();
extern "C" void isr7 ();
extern "C" void isr8 ();
extern "C" void isr9 ();
extern "C" void isr10 ();
extern "C" void isr11 ();
extern "C" void isr12 ();
extern "C" void isr13 ();
extern "C" void isr14 ();
extern "C" void isr15 ();
extern "C" void isr16 ();
extern "C" void isr17 ();
extern "C" void isr18 ();
extern "C" void isr19 ();
extern "C" void isr20 ();
extern "C" void isr21 ();
extern "C" void isr22 ();
extern "C" void isr23 ();
extern "C" void isr24 ();
extern "C" void isr25 ();
extern "C" void isr26 ();
extern "C" void isr27 ();
extern "C" void isr28 ();
extern "C" void isr29 ();
extern "C" void isr30 ();
extern "C" void isr31 ();

// IRQs
extern "C" void isr32 ();
extern "C" void isr33 ();
extern "C" void isr34 ();
extern "C" void isr35 ();
extern "C" void isr36 ();
extern "C" void isr37 ();
extern "C" void isr38 ();
extern "C" void isr39 ();
extern "C" void isr40 ();
extern "C" void isr41 ();
extern "C" void isr42 ();
extern "C" void isr43 ();
extern "C" void isr44 ();
extern "C" void isr45 ();
extern "C" void isr46 ();
extern "C" void isr47 ();

extern "C" void isr128 ();
extern "C" void isr255 ();


void IDT::init() {
    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);

    // Remapping IRQs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);


    // MMX/SSE
    uint32_t cr0;
    asm volatile(" mov %%cr0, %0": "=r"(cr0));
    cr0 &=0xFFFFFFF7;
    asm volatile(" mov %0, %%cr0":: "r"(cr0));


    setGate( 0, (uint32_t)isr0 , 0x08, 0x8E);
    setGate( 1, (uint32_t)isr1 , 0x08, 0x8E);
    setGate( 2, (uint32_t)isr2 , 0x08, 0x8E);
    setGate( 3, (uint32_t)isr3 , 0x08, 0x8E);
    setGate( 4, (uint32_t)isr4 , 0x08, 0x8E);
    setGate( 5, (uint32_t)isr5 , 0x08, 0x8E);
    setGate( 6, (uint32_t)isr6 , 0x08, 0x8E);
    setGate( 7, (uint32_t)isr7 , 0x08, 0x8E);
    setGate( 8, (uint32_t)isr8 , 0x08, 0x8E);
    setGate( 9, (uint32_t)isr9 , 0x08, 0x8E);
    setGate(10, (uint32_t)isr10, 0x08, 0x8E);
    setGate(11, (uint32_t)isr11, 0x08, 0x8E);
    setGate(12, (uint32_t)isr12, 0x08, 0x8E);
    setGate(13, (uint32_t)isr13, 0x08, 0x8E);
    setGate(14, (uint32_t)isr14, 0x08, 0x8E);
    setGate(15, (uint32_t)isr15, 0x08, 0x8E); //!
    setGate(16, (uint32_t)isr16, 0x08, 0x8E);
    setGate(17, (uint32_t)isr17, 0x08, 0x8E);
    setGate(18, (uint32_t)isr18, 0x08, 0x8E);
    setGate(19, (uint32_t)isr19, 0x08, 0x8E);
    setGate(20, (uint32_t)isr20, 0x08, 0x8E);
    setGate(21, (uint32_t)isr21, 0x08, 0x8E);
    setGate(22, (uint32_t)isr22, 0x08, 0x8E);
    setGate(23, (uint32_t)isr23, 0x08, 0x8E);
    setGate(24, (uint32_t)isr24, 0x08, 0x8E);
    setGate(25, (uint32_t)isr25, 0x08, 0x8E);
    setGate(26, (uint32_t)isr26, 0x08, 0x8E);
    setGate(27, (uint32_t)isr27, 0x08, 0x8E);
    setGate(28, (uint32_t)isr28, 0x08, 0x8E);
    setGate(29, (uint32_t)isr29, 0x08, 0x8E);
    setGate(30, (uint32_t)isr30, 0x08, 0x8E);
    setGate(31, (uint32_t)isr31, 0x08, 0x8E);
    setGate(32, (uint32_t)isr32, 0x08, 0x8E);
    setGate(33, (uint32_t)isr33, 0x08, 0x8E);
    setGate(34, (uint32_t)isr34, 0x08, 0x8E);
    setGate(35, (uint32_t)isr35, 0x08, 0x8E);
    setGate(36, (uint32_t)isr36, 0x08, 0x8E);
    setGate(37, (uint32_t)isr37, 0x08, 0x8E);
    setGate(38, (uint32_t)isr38, 0x08, 0x8E);
    setGate(39, (uint32_t)isr39, 0x08, 0x8E);
    setGate(40, (uint32_t)isr40, 0x08, 0x8E);
    setGate(41, (uint32_t)isr41, 0x08, 0x8E);
    setGate(42, (uint32_t)isr42, 0x08, 0x8E);
    setGate(43, (uint32_t)isr43, 0x08, 0x8E);
    setGate(44, (uint32_t)isr44, 0x08, 0x8E);
    setGate(45, (uint32_t)isr45, 0x08, 0x8E);
    setGate(46, (uint32_t)isr46, 0x08, 0x8E);
    setGate(47, (uint32_t)isr47, 0x08, 0x8E);
    setGate(128, (uint32_t)isr128, 0x08, 0x8E);
    setGate(255, (uint32_t)isr255, 0x08, 0x8E);

    flush();
}

void IDT::setGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags /* | 0x60 */;
}

void IDT::flush() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    _IDT_Flush((uint32_t)&idt_ptr);
}
