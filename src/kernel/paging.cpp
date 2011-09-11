#include "paging.h"
#include <util/cpp.h>
#include "kutils.h"
#include <memory/Heap.h>
#include <memory/Memory.h>
#include <interrupts/Interrupts.h>


extern void paging_alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    Memory::get()->allocate(page, is_kernel, is_writeable);
}

// Function to deallocate a frame.
extern void paging_free_frame(page_t *page)
{
    Memory::get()->free(page);
}





extern void paging_init()
{
    Memory::get()->startPaging();
}


extern void paging_switch_directory(AddressSpace *dir)
{
    Memory::get()->switchAddressSpace(dir);
}
