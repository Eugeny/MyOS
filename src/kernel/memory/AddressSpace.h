#ifndef MEMORY_ADDRESSPACE_H
#define MEMORY_ADDRESSPACE_H

#include <lang/lang.h>
#include <memory/Memory.h>


class AddressSpace {
public:
    AddressSpace();
    ~AddressSpace();

    static AddressSpace* kernelSpace;
    static AddressSpace* current;

    void _setRoot(page_tree_node_t* r);

    page_tree_node_t*       getRoot();
    void                    activate();
    void                    reset();
    AddressSpace*           clone();
    void                    release();
    page_tree_node_entry_t* getPage(uint64_t virt, bool create);
    uint64_t                getPhysicalAddress(uint64_t virt);
    page_tree_node_entry_t* allocatePage(uint64_t virt);
    void                    allocateSpace(uint64_t base, uint64_t size);
    void                    mapPage(uint64_t virt, uint64_t phy);
    void                    releasePage(uint64_t virt);
private:    
    page_tree_node_t *root;
};

#endif
