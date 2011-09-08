#ifndef MMU_H
#define MMU_H

#include "types.h"

extern u32int kalloc_get_boundary();
extern u32int kmalloc_a(u32int sz);
extern u32int kmalloc_p(u32int sz, u32int *phys);
extern u32int kmalloc_ap(u32int sz, u32int *phys);
extern u32int kmalloc(u32int sz);
extern void   kfree(u32int addr);
#endif 
