#include <memory/AddressSpace.h>
#include <memory/Heap.h>
#include <memory/Memory.h>
#include <paging.h>

extern "C" void copy_page_physical(u32int src ,  u32int dst);

AddressSpace::AddressSpace() {
    reset();
}

void AddressSpace::reset() {
    u32int phys;
    dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(dir, 0, sizeof(page_directory_t));
    u32int offset = (u32int)dir->tablesPhysical - (u32int)dir;
    dir->physicalAddr = phys + offset;
}

page_t *AddressSpace::getPage(u32int address, bool make) {

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

AddressSpace* AddressSpace::clone()
{
AddressSpace* n = new AddressSpace();
    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!dir->tables[i])
            continue;

        if (Memory::get()->getKernelSpace()->dir->tables[i] == dir->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            n->dir->tables[i] = dir->tables[i];
            n->dir->tablesPhysical[i] = dir->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            u32int phys;
            n->dir->tables[i] = clone_table(dir->tables[i], &phys);
            n->dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return n;
}
