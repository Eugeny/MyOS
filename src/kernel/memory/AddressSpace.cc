#include <memory/AddressSpace.h>
#include <memory/Memory.h>
#include <kutil.h>


AddressSpace* AddressSpace::kernelSpace = NULL;
AddressSpace* AddressSpace::current = NULL;


AddressSpace::AddressSpace() {
}

AddressSpace::~AddressSpace() {
    delete root;
}

void AddressSpace::activate() {
    memory_load_page_tree(root);
    current = this;
}

page_tree_node_t* AddressSpace::getRoot() {
    return root;
}

void AddressSpace::_setRoot(page_tree_node_t* r) {
    root = r;
}

page_tree_node_entry_t* AddressSpace::getPage(uint64_t virt, bool create) {
    return memory_get_page(root, virt, create);
}

page_tree_node_entry_t *AddressSpace::allocatePage(uint64_t virt) {
    page_tree_node_entry_t* page = getPage(virt, false);
    klog('t', "Allocating vaddr %lx", virt);
    if (!page) {
        page = getPage(virt, true);
    }
    if (!page->present) {
        Memory::get()->allocatePage(page);
    }
    return page;
}

void AddressSpace::allocateSpace(uint64_t base, uint64_t size) {
    base = base / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    size = (size + 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    for (uint64_t v = base; v < base + size; v += KCFG_PAGE_SIZE) {
        allocatePage(v);
    }
}


void AddressSpace::mapPage(uint64_t virt, uint64_t phy) {
    klog('t', "Mapping page: %lx -> %lx", virt, phy);
    page_tree_node_entry_t* page = getPage(virt, true);
    Memory::get()->mapPage(page, phy);
}

void AddressSpace::releasePage(uint64_t virt) {
    page_tree_node_entry_t* page = getPage(virt, false);
    if (page)
        Memory::get()->releasePage(page);
}

void AddressSpace::release() {
}
