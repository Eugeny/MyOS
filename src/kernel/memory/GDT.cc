#include <memory/GDT.h>
#include <string.h>


#define GDT_SIZE 7


typedef struct gdt_entry_struct
{
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, determine what ring this segment can be used in.
   uint8_t  granularity;
   uint8_t  base_high;           // The last 8 bits of the base.
} PACKED gdt_entry_t;


typedef struct gdt_ptr_struct {
   uint16_t limit;               // The upper 16 bits of all selector limits.
   uint32_t base;                // The address of the first gdt_entry_t struct.
} PACKED gdt_ptr_t;


static gdt_entry_t ALIGN(16) gdt_entries[GDT_SIZE];
static gdt_ptr_t   ALIGN(16) gdt_ptr;

extern "C" void _GDT_flush(uint32_t dsc);


void GDT::init() {
    memset(&gdt_entries, 0, sizeof(gdt_entry_t)*GDT_SIZE);
}

void GDT::setGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
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
    gdt_ptr.base  = (uint32_t)&gdt_entries;
    uint32_t descriptor = (uint32_t)&gdt_ptr;
    _GDT_flush(descriptor);
}
