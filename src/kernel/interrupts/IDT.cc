#include <interrupts/IDT.h>
#include <util/cpp.h>


typedef struct idt_entry_struct {
    u16int base_lo;
    u16int sel;
    u8int always0;
    u8int flags;
    u16int base_hi;
} PACKED idt_entry_t;


typedef struct idt_ptr_struct {
    u16int limit;
    u32int base;
} PACKED idt_ptr_t;


static idt_entry_t ALIGN(16) idt_entries[256];
static idt_ptr_t   ALIGN(16) idt_ptr;

extern "C" void _IDT_Flush(u32int);


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

    setGate( 0, (u32int)isr0 , 0x08, 0x8E);
    setGate( 1, (u32int)isr1 , 0x08, 0x8E);
    setGate( 2, (u32int)isr2 , 0x08, 0x8E);
    setGate( 3, (u32int)isr3 , 0x08, 0x8E);
    setGate( 4, (u32int)isr4 , 0x08, 0x8E);
    setGate( 5, (u32int)isr5 , 0x08, 0x8E);
    setGate( 6, (u32int)isr6 , 0x08, 0x8E);
    setGate( 7, (u32int)isr7 , 0x08, 0x8E);
    setGate( 8, (u32int)isr8 , 0x08, 0x8E);
    setGate( 9, (u32int)isr9 , 0x08, 0x8E);
    setGate(10, (u32int)isr10, 0x08, 0x8E);
    setGate(11, (u32int)isr11, 0x08, 0x8E);
    setGate(12, (u32int)isr12, 0x08, 0x8E);
    setGate(13, (u32int)isr13, 0x08, 0x8E);
    setGate(14, (u32int)isr14, 0x08, 0x8E);
    setGate(15, (u32int)isr15, 0x08, 0x8E); //!
    setGate(16, (u32int)isr16, 0x08, 0x8E);
    setGate(17, (u32int)isr17, 0x08, 0x8E);
    setGate(18, (u32int)isr18, 0x08, 0x8E);
    setGate(19, (u32int)isr19, 0x08, 0x8E);
    setGate(20, (u32int)isr20, 0x08, 0x8E);
    setGate(21, (u32int)isr21, 0x08, 0x8E);
    setGate(22, (u32int)isr22, 0x08, 0x8E);
    setGate(23, (u32int)isr23, 0x08, 0x8E);
    setGate(24, (u32int)isr24, 0x08, 0x8E);
    setGate(25, (u32int)isr25, 0x08, 0x8E);
    setGate(26, (u32int)isr26, 0x08, 0x8E);
    setGate(27, (u32int)isr27, 0x08, 0x8E);
    setGate(28, (u32int)isr28, 0x08, 0x8E);
    setGate(29, (u32int)isr29, 0x08, 0x8E);
    setGate(30, (u32int)isr30, 0x08, 0x8E);
    setGate(31, (u32int)isr31, 0x08, 0x8E);
    setGate(32, (u32int)isr32, 0x08, 0x8E);
    setGate(33, (u32int)isr33, 0x08, 0x8E);
    setGate(34, (u32int)isr34, 0x08, 0x8E);
    setGate(35, (u32int)isr35, 0x08, 0x8E);
    setGate(36, (u32int)isr36, 0x08, 0x8E);
    setGate(37, (u32int)isr37, 0x08, 0x8E);
    setGate(38, (u32int)isr38, 0x08, 0x8E);
    setGate(39, (u32int)isr39, 0x08, 0x8E);
    setGate(40, (u32int)isr40, 0x08, 0x8E);
    setGate(41, (u32int)isr41, 0x08, 0x8E);
    setGate(42, (u32int)isr42, 0x08, 0x8E);
    setGate(43, (u32int)isr43, 0x08, 0x8E);
    setGate(44, (u32int)isr44, 0x08, 0x8E);
    setGate(45, (u32int)isr45, 0x08, 0x8E);
    setGate(46, (u32int)isr46, 0x08, 0x8E);
    setGate(47, (u32int)isr47, 0x08, 0x8E);

    flush();
}

void IDT::setGate(u8int num, u32int base, u16int sel, u8int flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags /* | 0x60 */;
}

void IDT::flush() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (u32int)&idt_entries;

    _IDT_Flush((u32int)&idt_ptr);
}
