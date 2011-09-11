#ifndef PAGING_H
#define PAGING_H

#include <util/cpp.h>
#include <memory/AddressSpace.h>



extern AddressSpace *kernel_directory;


typedef struct memory_info {
    u32int total_frames;
    u32int used_frames;
} memory_info_t;

/**
  Sets up the environment, page directories etc and
  enables paging.
**/
extern void paging_init();

extern void paging_info(memory_info_t*);

extern void paging_alloc_frame(page_t *page, int is_kernel, int is_writeable);

extern void paging_free_frame(page_t *page);

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
extern void paging_switch_directory(AddressSpace *n);



#endif
