#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <lang/lang.h>
#include <lang/Singleton.h>


struct page_tree_node_entry_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t address    : 52;  // Frame address (shifted right 12 bits)
};
 

struct page_tree_node_t {
    page_tree_node_entry_t entries[512];
};


void memory_initialize_default_paging();
page_tree_node_entry_t* memory_get_page(page_tree_node_t* root, uint64_t virt, bool create);
void memory_map_page(page_tree_node_t* root, uint64_t virt, uint64_t phys);
void memory_load_page_tree(page_tree_node_t* root);


class Memory : public Singleton<Memory> {
public:
    void allocatePage(page_tree_node_entry_t* page);
    void releasePage(page_tree_node_entry_t* page);
private:    
};
#endif
