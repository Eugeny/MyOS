#include "paging.h"
#include "types.h"
#include "kutils.h"
#include "kalloc.h"
#include "isr.h"

page_directory_t *kernel_directory=0;
page_directory_t *current_directory=0;


u32int *frames;
u32int nframes;
u32int used_frames = 0;

extern "C" void copy_page_physical(u32int src ,  u32int dst);
static page_directory_t* clone_directory(page_directory_t*);

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


extern void paging_alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
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
       page->rw = (is_writeable)?1:0; // Should the page be writeable?
       page->user = (is_kernel)?0:1; // Should the page be user-mode?
       page->frame = idx;
   }
}

// Function to deallocate a frame.
extern void paging_free_frame(page_t *page)
{
   u32int frame;
   if (!(frame=page->frame))
   {
       return; // The given page didn't actually have an allocated frame!
   }
   else
   {
       clear_frame(frame); // Frame is now free again.
       page->frame = 0x0; // Page now doesn't have a frame.
   }
}




u32int get_total_ram() {
    return 512 * 1024 * 1024;
}


void paging_cover_kernel() {
   u32int frame = 0;
   u32int bound = 0xFFFFFFFF;
   while (frame < bound)
   {
       paging_alloc_frame( paging_get_page(frame, 1, kernel_directory), 0, 0);
       frame += 0x1000;
       bound = kalloc_get_boundary();
   }
}

extern void paging_init()
{
   u32int mem_end_page = get_total_ram();

   nframes = mem_end_page / 0x1000;
   frames = (u32int*)kmalloc(BS_IDX(nframes));
   memset(frames, 0, BS_IDX(nframes));
   // Let's make a page directory.
   kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
   memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (u32int)kernel_directory->tablesPhysical;

   // We need to identity map (phys addr = virt addr) from
   // 0x0 to the end of used memory, so we can access this
   // transparently, as if paging wasn't enabled.
   // NOTE that we use a while loop here deliberately.
   // inside the loop body we actually change placement_address
   // by calling kmalloc(). A while loop causes this to be
   // computed on-the-fly rather than once at the start.

    kheap_init();
    kheap_map_pages();
    paging_cover_kernel();
    kheap_alloc_pages();
    kheap_enable();
    
   // Before we enable paging, we must register our page fault handler.
    set_interrupt_handler(14, page_fault);

    paging_switch_directory(kernel_directory);
    current_directory = clone_directory(kernel_directory);
    paging_switch_directory (current_directory);
}

extern void paging_info(memory_info_t* i) {
    i->total_frames = nframes;
    i->used_frames = used_frames;
}

extern void paging_switch_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(dir->physicalAddr));
   u32int cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

extern page_t *paging_get_page(u32int address, int make, page_directory_t *dir)
{
   // Turn the address into an index.
   address /= 0x1000;
   // Find the page table containing this address.
   u32int table_idx = address / 1024;
   if (dir->tables[table_idx]) // If this table is already assigned
   {
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else if(make)
   {
       u32int tmp;
       dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else
   {
       return 0;
   }
}


void page_fault(registers_t regs)
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
   if (rw) {klogn("ro ");}
   if (us) {klogn("user ");}
   if (reserved) {klogn("reserved ");}
   klogn(" at ");
   klog(to_hex(faulting_address));
   PANIC("Page fault");
}



static page_table_t *clone_table(page_table_t *src, u32int *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);
    // Ensure that the new table is blank.
    memset(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (src->pages[i].frame)
        {
            // Get a new frame.
            paging_alloc_frame(&table->pages[i], 0, 0);
            // Clone the flags from source to destination.
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            // Physically copy the data across. This function is in process.s.
            copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
        }
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    u32int phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    // Ensure that it is blank.
    memset(dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    u32int offset = (u32int)dir->tablesPhysical - (u32int)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            u32int phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}
