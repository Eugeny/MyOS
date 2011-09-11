#include <util/cpp.h>
#include <memory/Memory.h>
#include <memory/Heap.h>
#include <interrupts/Interrupts.h>
#include "kutils.h"


u32int *frames;
u32int nframes;
u32int used_frames = 0;

#define BS_IDX(a) (a/32)
#define BS_OFF(a) (a%32)

// Static function to set a bit in the frames bitset
static void set_frame(u32int frame) {
    frame /= 0x1000;
    u32int idx = BS_IDX(frame);
    u8int  off = BS_OFF(frame);
    if (!frames[idx] & (0x1 << off))
        used_frames++;
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(u32int frame) {
    frame /= 0x1000;
    u32int idx = BS_IDX(frame);
    u8int  off = BS_OFF(frame);
    if (frames[idx] & (0x1 << off))
        used_frames--;
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static u32int test_frame(u32int frame) {
   frame /= 0x1000;
   u32int idx = BS_IDX(frame);
   u8int  off = BS_OFF(frame);
   return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static u32int first_frame()
{
   for (u32int i = 0; i < BS_IDX(nframes); i++)
       if (frames[i] != 0xFFFFFFFF)
           for (u32int j = 0; j < 32; j++)
               if (!(frames[i] & (1 << j)))
                   return i*32+j;
   return 0xFFFFFFFF;
}


void page_fault(isrq_registers_t regs);


u32int get_total_ram() {
    return 512 * 1024 * 1024;
}



void Memory::allocate(page_t* page, bool kernel, bool rw) {

   if (page->frame != 0)
   {
       return; // Frame was already allocated, return straight away.
   }
   else
   {
       u32int idx = first_frame(); // idx is now the index of the first free frame.
       if (idx == (u32int)-1)
       {
           // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
           PANIC("No free frames!");
       }
       set_frame(idx*0x1000); // this frame is now ours!
       page->present = 1; // Mark it as present.
       page->rw = (rw)?1:0; // Should the page be writeable?
       page->user = (kernel)?0:1; // Should the page be user-mode?
       page->frame = idx;
   }
   }

void Memory::free(page_t* page) {

   u32int frame;
   if (!(frame=page->frame))
   {
       return; // The given page didn't actually have an allocated frame!
   }
   else
   {
       clear_frame(frame); // Frame is now free again.
       page->frame = 0x0; // Page now doesn't have a frame.
   }}


void Memory::startPaging() {
   u32int mem_end_page = get_total_ram();
TRACE
   nframes = mem_end_page / 0x1000;
   frames = (u32int*)kmalloc(BS_IDX(nframes));
   memset(frames, 0, BS_IDX(nframes));
   // Let's make a page directory.
   kernelSpace = new AddressSpace();
   kernelSpace->dir->physicalAddr = (u32int)kernelSpace->dir->tablesPhysical; //?
TRACE

   // We need to identity map (phys addr = virt addr) from
   // 0x0 to the end of used memory, so we can access this
   // transparently, as if paging wasn't enabled.
   // NOTE that we use a while loop here deliberately.
   // inside the loop body we actually change placement_address
   // by calling kmalloc(). A while loop causes this to be
   // computed on-the-fly rather than once at the start.

TRACE
    Heap::get()->switchToHeap();
TRACE

   // Before we enable paging, we must register our page fault handler.
    Interrupts::get()->setHandler(14, page_fault);

    switchAddressSpace(kernelSpace);
    currentSpace = kernelSpace->clone();
    switchAddressSpace (currentSpace);
}


void Memory::setAddressSpace(AddressSpace *s) {
    currentSpace = s;
}


void Memory::switchAddressSpace(AddressSpace *s) {
    currentSpace = s;
    asm volatile("mov %0, %%cr3":: "r"(s->dir->physicalAddr));
    u32int cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}


u32int Memory::getTotalFrames() {
    return nframes;
}

u32int Memory::getUsedFrames() {
    return used_frames;
}

AddressSpace* Memory::getKernelSpace() {
    return kernelSpace;
}

AddressSpace* Memory::getCurrentSpace() {
    return currentSpace;
}



void page_fault(isrq_registers_t regs)
{
   u32int faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   int present   = !(regs.err_code & 0x1); // Page not present
   int rw = regs.err_code & 0x2;           // Write operation?
   int us = regs.err_code & 0x4;           // Processor was in user-mode?
   int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
   klogn("PAGE FAULT: ");
   if (present) {klogn("notpresent ");}
   if (rw) {klogn("write ");}
   if (us) {klogn("user ");}
   if (reserved) {klogn("reserved ");}
   if (id) {klogn("exec ");}
   klogn(" at ");
   klog(to_hex(faulting_address));
   PANIC("Page fault");
}