#include <interrupts/IDT.h>
#include <hardware/io.h>
#include <string.h>
#include <kutil.h>


typedef struct idt_entry_struct {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t zero0;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_hi;
    uint32_t zero1;
} PACKED idt_entry_t;


typedef struct idt_ptr_struct {
    uint16_t limit;
    uint64_t base;
} PACKED idt_ptr_t;


static idt_entry_t ALIGN(64) idt_entries[256];
static idt_ptr_t   ALIGN(64) idt_ptr;

extern "C" void _IDT_Flush(uint64_t);


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
    //memset(&idt_entries, 0, sizeof(idt_entry_t)*256);

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
    uint64_t cr0;
    asm volatile(" mov %%cr0, %0": "=r"(cr0));
    cr0 &=0xFFFFFFF7;
    asm volatile(" mov %0, %%cr0":: "r"(cr0));

    uint16_t selector = 3 << 3;

    setGate( 0, (uint64_t)isr0 , selector, 0x8E);
    setGate( 1, (uint64_t)isr1 , selector, 0x8E);
    setGate( 2, (uint64_t)isr2 , selector, 0x8E);
    setGate( 3, (uint64_t)isr3 , selector, 0x8E);
    setGate( 4, (uint64_t)isr4 , selector, 0x8E);
    setGate( 5, (uint64_t)isr5 , selector, 0x8E);
    setGate( 6, (uint64_t)isr6 , selector, 0x8E);
    setGate( 7, (uint64_t)isr7 , selector, 0x8E);
    setGate( 8, (uint64_t)isr8 , selector, 0x8E);
    setGate( 9, (uint64_t)isr9 , selector, 0x8E);
    setGate(10, (uint64_t)isr10, selector, 0x8E);
    setGate(11, (uint64_t)isr11, selector, 0x8E);
    setGate(12, (uint64_t)isr12, selector, 0x8E);
    setGate(13, (uint64_t)isr13, selector, 0x8E);
    setGate(14, (uint64_t)isr14, selector, 0x8E);
    setGate(15, (uint64_t)isr15, selector, 0x8E); //!
    setGate(16, (uint64_t)isr16, selector, 0x8E);
    setGate(17, (uint64_t)isr17, selector, 0x8E);
    setGate(18, (uint64_t)isr18, selector, 0x8E);
    setGate(19, (uint64_t)isr19, selector, 0x8E);
    setGate(20, (uint64_t)isr20, selector, 0x8E);
    setGate(21, (uint64_t)isr21, selector, 0x8E);
    setGate(22, (uint64_t)isr22, selector, 0x8E);
    setGate(23, (uint64_t)isr23, selector, 0x8E);
    setGate(24, (uint64_t)isr24, selector, 0x8E);
    setGate(25, (uint64_t)isr25, selector, 0x8E);
    setGate(26, (uint64_t)isr26, selector, 0x8E);
    setGate(27, (uint64_t)isr27, selector, 0x8E);
    setGate(28, (uint64_t)isr28, selector, 0x8E);
    setGate(29, (uint64_t)isr29, selector, 0x8E);
    setGate(30, (uint64_t)isr30, selector, 0x8E);
    setGate(31, (uint64_t)isr31, selector, 0x8E);
    setGate(32, (uint64_t)isr32, selector, 0x8E);
    setGate(33, (uint64_t)isr33, selector, 0x8E);
    setGate(34, (uint64_t)isr34, selector, 0x8E);
    setGate(35, (uint64_t)isr35, selector, 0x8E);
    setGate(36, (uint64_t)isr36, selector, 0x8E);
    setGate(37, (uint64_t)isr37, selector, 0x8E);
    setGate(38, (uint64_t)isr38, selector, 0x8E);
    setGate(39, (uint64_t)isr39, selector, 0x8E);
    setGate(40, (uint64_t)isr40, selector, 0x8E);
    setGate(41, (uint64_t)isr41, selector, 0x8E);
    setGate(42, (uint64_t)isr42, selector, 0x8E);
    setGate(43, (uint64_t)isr43, selector, 0x8E);
    setGate(44, (uint64_t)isr44, selector, 0x8E);
    setGate(45, (uint64_t)isr45, selector, 0x8E);
    setGate(46, (uint64_t)isr46, selector, 0x8E);
    setGate(47, (uint64_t)isr47, selector, 0x8E);
    setGate(128, (uint64_t)isr128, selector, 0x8E);
    setGate(255, (uint64_t)isr255, selector, 0x8E);

    flush();
    sout("TRACE");for(;;);
}

void IDT::setGate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
    idt_entries[num].base_hi = (base >> 32) & 0xFFFFFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].zero0   = 0;
    idt_entries[num].zero1   = 0;
    idt_entries[num].flags   = flags /* | 0x60 */;
}

void IDT::flush() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (uint64_t)&idt_entries;

    _IDT_Flush((uint64_t)&idt_ptr);
}
