#include <memory/GDT.h>
#include <util/cpp.h>


#define GDT_SIZE 5


typedef struct gdt_entry_struct
{
   u16int limit_low;           // The lower 16 bits of the limit.
   u16int base_low;            // The lower 16 bits of the base.
   u8int  base_middle;         // The next 8 bits of the base.
   u8int  access;              // Access flags, determine what ring this segment can be used in.
   u8int  granularity;
   u8int  base_high;           // The last 8 bits of the base.
} PACKED gdt_entry_t;


typedef struct gdt_ptr_struct {
   u16int limit;               // The upper 16 bits of all selector limits.
   u32int base;                // The address of the first gdt_entry_t struct.
} PACKED gdt_ptr_t;


static gdt_entry_t ALIGN(16) gdt_entries[GDT_SIZE];
static gdt_ptr_t   ALIGN(16) gdt_ptr;

extern "C" void _GDT_flush(u32int dsc);


void GDT::init() {
    memset(&gdt_entries, 0, sizeof(gdt_entry_t)*GDT_SIZE);
}

void GDT::setGate(s32int num, u32int base, u32int limit, u8int access, u8int gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void GDT::setDefaults() {
   setGate(0, 0, 0, 0, 0);                // Null segment
   setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
   setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
   setGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
   setGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

   flush();
}

void GDT::flush() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
    gdt_ptr.base  = (u32int)&gdt_entries;
    u32int descriptor = (u32int)&gdt_ptr;
    _GDT_flush(descriptor);
}
