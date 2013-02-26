#include <memory/AddressSpace.h>
#include <memory/Memory.h>
#include <kutil.h>


AddressSpace::AddressSpace() {
}

AddressSpace::~AddressSpace() {
    delete root;
}

void AddressSpace::_setRoot(page_tree_node_t* r) {
    root = r;
}

page_tree_node_entry_t* AddressSpace::getPage(uint64_t virt, bool create) {
    return memory_get_page(root, virt, create);
}

page_tree_node_entry_t *AddressSpace::allocatePage(uint64_t virt) {
    page_tree_node_entry_t* page = getPage(virt, true);
    Memory::get()->allocatePage(page);
    return page;
}

void AddressSpace::releasePage(uint64_t virt) {
    page_tree_node_entry_t* page = getPage(virt, false);
    if (page)
        Memory::get()->releasePage(page);
}

void AddressSpace::release() {
}
