#ifndef MEMORY_ADDRESSPACE_H
#define MEMORY_ADDRESSPACE_H

#include <lang/lang.h>
#include <memory/Memory.h>




struct page_tree_node_entry_t {
    uint64_t present    : 1;   // Page present in memory
    uint64_t rw         : 1;   // Read-only if clear, readwrite if set
    uint64_t user       : 1;   // Supervisor level only if clear
    uint64_t unused     : 9;   // Amalgamation of unused and reserved bits
    uint64_t address    : 52;  // Frame address (shifted right 12 bits)
};
 
struct page_tree_node_t {
    page_tree_node_entry_t entries[512];
    page_tree_node_t* entriesVirtual[512];
    char*   entriesNames[512];
    uint8_t entriesAttrs[512];
};

struct page_descriptor_t {
    page_tree_node_entry_t* entry;
    page_tree_node_t** vAddr;
    uint8_t* attrs;
    char**   name;
    uint64_t pageVAddr;
};

#define PAGEATTR_SHARED 1
#define PAGEATTR_USER 2
#define PAGEATTR_COPY 4
#define PAGEATTR_IS_SHARED(a)   ((a & PAGEATTR_SHARED) != 0)
#define PAGEATTR_IS_USER(a)     ((a & PAGEATTR_USER) != 0)
#define PAGEATTR_IS_COPY(a)     ((a & PAGEATTR_COPY) != 0)
 
#define PAGE_INDEX(virt) (virt / KCFG_PAGE_SIZE % 512)





class AddressSpace {
public:
    AddressSpace();
    ~AddressSpace();

    static AddressSpace* kernelSpace;
    static AddressSpace* current;

    void                    initEmpty();

    page_tree_node_t*       getRoot();
    void                    setRoot(page_tree_node_t* r);
    void                    activate();
    void                    reset();
    AddressSpace*           clone();
    void                    release();

    page_descriptor_t       getPage(uint64_t virt, bool create);
    uint64_t                getPhysicalAddress(uint64_t virt);
    page_descriptor_t       mapPage(page_descriptor_t page, uint64_t phy, uint8_t attrs);
    void                    namePage(page_descriptor_t page, char* name);
    page_descriptor_t       allocatePage(page_descriptor_t page, uint8_t attrs);
    void                    allocateSpace(uint64_t base, uint64_t size, uint8_t attrs);
    void                    releasePage(page_descriptor_t page);
    void                    releaseSpace(uint64_t base, uint64_t size);
    
    void                    dump();
private:    
    void                    recursiveDump(page_tree_node_t* node, int level);
    page_tree_node_t *root;
};

#endif
