#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <lang/lang.h>


typedef struct page_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t frame      : 52;  // Frame address (shifted right 12 bits)
};


typedef struct page_table_t {
    page_t entries[512];
};


typedef struct page_directory_entry_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t table      : 52;
};


typedef struct page_directory_t {
    page_directory_entry_t entries[512];
};


typedef struct pdpt_entry_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t table      : 52;
};


typedef struct pdpt_t {
    pdpt_entry_t entries[512];
};


typedef struct pml4_entry_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t table      : 52;
};


typedef struct pml4_t {
    pml4_entry_t entries[512];
};

#endif
