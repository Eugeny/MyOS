#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <lang/lang.h>


struct page_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t address    : 52;  // Frame address (shifted right 12 bits)
};
 

struct page_tree_node_t {
    page_t entries[512];
};



void memory_initialize_default_paging();

#endif
