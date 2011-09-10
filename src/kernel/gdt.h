#ifndef GDT_H
#define GDT_H

#include <util/cpp.h>

#define GDT_SIZE 256

struct gdt_entry_struct
{
   u16int limit_low;           // The lower 16 bits of the limit.
   u16int base_low;            // The lower 16 bits of the base.
   u8int  base_middle;         // The next 8 bits of the base.
   u8int  access;              // Access flags, determine what ring this segment can be used in.
   u8int  granularity;
   u8int  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
   u16int limit;               // The upper 16 bits of all selector limits.
   u32int base;                // The address of the first gdt_entry_t struct.
}
 __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;


extern "C" void gdt_flush(u32int);
extern void gdt_set_gate(s32int, u32int, u32int, u8int, u8int);
extern void gdt_init();

static const int GDT_ACC = (1 << 4) + (1 << 7);
static const int GDT_ACC_PRIV_3 = 3 << 5;
static const int GDT_ACC_PRIV_0 = 0;
static const int GDT_ACC_TYPE_CODE = 1 << 3;
static const int GDT_ACC_TYPE_DATA = 0 << 3;
static const int GDT_ACC_CODE_CONFORM = 1 << 2;
static const int GDT_ACC_RW = 2;




#endif 
