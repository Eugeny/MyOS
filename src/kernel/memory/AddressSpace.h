#ifndef MEMORY_ADDRESSSPACE_H
#define MEMORY_ADDRESSSPACE_H

#include <util/cpp.h>
typedef struct page {
    u32int present    : 1;   // Page present in memory
    u32int rw         : 1;   // Read-only if clear, readwrite if set
    u32int user       : 1;   // Supervisor level only if clear
    u32int accessed   : 1;   // Has the page been accessed since last refresh?
    u32int dirty      : 1;   // Has the page been written to since last refresh?
    u32int unused     : 7;   // Amalgamation of unused and reserved bits
    u32int frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];
    u32int tablesPhysical[1024];
    u32int physicalAddr;
} page_directory_t;


class AddressSpace {
public:
    AddressSpace();
    void          reset();
    AddressSpace* clone();
    page_t*       getPage(u32int addr, bool create);
    page_t*       allocatePage(u32int addr, bool create, bool kernel, bool rw);
    page_directory_t *dir;
};

#endif
