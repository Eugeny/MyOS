#ifndef MEMORY_ADDRESSSPACE_H
#define MEMORY_ADDRESSSPACE_H

#include <lang/lang.h>
#include <memory/Memory.h>


class AddressSpace {
public:
    AddressSpace();
    ~AddressSpace();

    void _setRoot(page_tree_node_t* r);

    void                    reset();
    AddressSpace*           clone();
    void                    release();
    page_tree_node_entry_t* getPage(uint64_t virt, bool create);
    page_tree_node_entry_t* allocatePage(uint64_t virt);
    void                    releasePage(uint64_t virt);
private:    
    page_tree_node_t *root;
};

#endif
