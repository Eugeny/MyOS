#include <alloc/malloc.h>
#include <core/CPU.h>
#include <memory/AddressSpace.h>
#include <memory/FrameAlloc.h>
#include <memory/Memory.h>
#include <kutil.h>



// -----------------------------
// Page tree operations

page_tree_node_t* __initial_pml4;

static uint64_t __allocate_heap(uint64_t sz) {
    sz = ((sz + (KCFG_PAGE_SIZE - 1)) / KCFG_PAGE_SIZE) * KCFG_PAGE_SIZE;
    return (uint64_t)kvalloc(sz);
}


static page_tree_node_t* allocate_node() {
    return (page_tree_node_t*)__allocate_heap(sizeof(page_tree_node_t));
}

static void initialize_node_entry(page_tree_node_entry_t* entry) {
    entry->present = 0;
    entry->rw = 1;
    entry->user = 1;
    entry->unused = 0;
    entry->address = 0xcccccccc; // trap
}

static void initialize_node(page_tree_node_t* node) {
    for (int idx = 0; idx < 512; idx++)
        initialize_node_entry(&(node->entries[idx]));
}

static page_tree_node_t* node_get_child(page_tree_node_t* node, uint64_t idx, bool create) {
    //__outputhex((uint64_t)__initial_heap,60);
    //for (int c = 0; c < 32000; c++);
    //__outputhex((uint64_t)node,70);
    if ((uint64_t)node > 0x30000){
    //for (int c = 0; c < 3200000; c++);
    }

    if (!node->entries[idx].present) {
        if (!create) {
            return NULL;
        }
        page_tree_node_t* child = allocate_node();
        initialize_node(child);
        node->entries[idx].present = 1;
        node->entries[idx].address = (uint64_t)child / KCFG_PAGE_SIZE; 
        return child;
    }
    return (page_tree_node_t*)(node->entries[idx].address * KCFG_PAGE_SIZE);
}

// -----------------------------

page_tree_node_entry_t* memory_get_page(page_tree_node_t* root, uint64_t virt, bool create) {
    // Skip memory hole
    if (virt >= 0xffff800000000000) {
        //__outputhex((uint64_t)virt ,50);
        virt -= 0xffff000000000000;
    }

    virt = virt / KCFG_PAGE_SIZE; // page index
    uint64_t indexes[4] = {
        virt / 512 / 512 / 512 % 512,
        virt / 512 / 512 % 512,
        virt / 512 % 512,
        virt % 512,
    };

    for (int i = 0; i < 3; i++) {
        root = node_get_child(root, indexes[i], create);
        if (root == NULL && !create)
            return 0;
    }
    
    int index = indexes[3];
    return &(root->entries[index]);
}

void memory_map_page(page_tree_node_t* root, uint64_t virt, uint64_t phys) {
    page_tree_node_entry_t* page = memory_get_page(root, virt, true);
    page->present = 1;
    page->rw = 1;
    page->user = 1;
    page->address = phys / KCFG_PAGE_SIZE;
}

void memory_load_page_tree(page_tree_node_t* root) {
    CPU::setCR3((uint64_t)root);
    /*uint64_t v;
    asm volatile("mov %%cr0, %0": "=r"(v));
    v &= 0x7FFFFFFF;
    asm volatile("mov %0, %%cr0":: "r"(v));
    v |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(v));
    asm volatile("mov %%cr3, %0": "=r"(v));
    asm volatile("mov %0, %%cr3":: "r"(v));*/
    //CPU::setCR0(CPU::getCR0() & 0x7FFFFFFF);
    //CPU::setCR3((uint64_t)0);
    //CPU::setCR3((uint64_t)root);
    //CPU::setCR0(CPU::getCR0() | 0x80000000);
}

void memory_initialize_default_paging() {
    __initial_pml4 = (page_tree_node_t*) 0x20000;
    initialize_node(__initial_pml4);

    FrameAlloc::get()->init((512*1024*1024) / KCFG_PAGE_SIZE);
    //__initial_heap = (uint64_t)__initial_pml4 + sizeof(page_tree_node_t);
    
    
    for (uint64_t i = 0; i < KCFG_LOW_IDENTITY_PAGING_LENGTH; i += KCFG_PAGE_SIZE) {
        memory_map_page(__initial_pml4, i, i);
        FrameAlloc::get()->markAllocated(i / KCFG_PAGE_SIZE);
    }

    for (uint64_t i = 0; i <= KCFG_HIGH_IDENTITY_PAGING_LENGTH; i += KCFG_PAGE_SIZE) { 
        memory_map_page(__initial_pml4, 0xffffffffffffffff - KCFG_HIGH_IDENTITY_PAGING_LENGTH + i, KCFG_LOW_IDENTITY_PAGING_LENGTH + i);
        FrameAlloc::get()->markAllocated((KCFG_LOW_IDENTITY_PAGING_LENGTH + i) / KCFG_PAGE_SIZE);
    }

    memory_load_page_tree(__initial_pml4);
}



void Memory::init() {
    AddressSpace* kernelSpace = new AddressSpace();
    kernelSpace->_setRoot(__initial_pml4);
    AddressSpace::kernelSpace = kernelSpace;
    kernelSpace->activate();
}

void Memory::allocatePage(page_tree_node_entry_t* page) {
    mapPage(page, FrameAlloc::get()->allocate() * KCFG_PAGE_SIZE);
}

void Memory::mapPage(page_tree_node_entry_t* page, uint64_t phy) {
    klog('t', "Mapping page %lx -> %lx",page, phy);
    page->address = phy / KCFG_PAGE_SIZE;
    page->present = true;
}

void Memory::releasePage(page_tree_node_entry_t* page) {
    initialize_node_entry(page);
    page->present = false;
}

void Memory::handlePageFault(isrq_registers_t* reg) {
    dump(AddressSpace::current->getRoot());

    const char* fPresent  = (reg->err_code & 1) ? "P" : "-";
    const char* fWrite    = (reg->err_code & 2) ? "W" : "-";
    const char* fUser     = (reg->err_code & 4) ? "U" : "-";
    const char* fRW       = (reg->err_code & 8) ? "R" : "-";
    const char* fIFetch   = (reg->err_code & 16) ? "I" : "-";
    klog('e', "PAGE FAULT [%s%s%s%s%s]", fPresent, fWrite, fUser, fRW, fIFetch);
    klog('e', "Faulting address : %lx", CPU::getCR2());
    klog('e', "Faulting code    : %lx", reg->rip);
    klog_flush();
    for(;;);
}


static void recursiveDump(page_tree_node_t* node, int level) {
    static uint64_t skips[4] = {
        512 * 512 * 512,
        512 * 512,
        512
    };

    static uint64_t addr, startPhy, startVirt, lastVirt, lastPhy, len;
    static bool started = false;

    if (level == 0) {
        addr = 0;
        startPhy = 0;
        startVirt = 0;
        len = 0;
        started = false;
        lastVirt = -1;
    }

    if (!node) {
        for(;;);
    }
    for (int i = 0; i < ((level == 0) ? 512 : 512); i++) {
        if (addr >= 0x0000800000000000 && addr < 0xffff800000000000)
            addr += 0xffff000000000000;

        if (level == 3) {
            if (node->entries[i].present) {
                startVirt = addr;
                startPhy = node->entries[i].address * KCFG_PAGE_SIZE;
                if ((startVirt != lastVirt + KCFG_PAGE_SIZE) || (startPhy != lastPhy + KCFG_PAGE_SIZE)) {
                    if (started)
                        klog('i', " == %lx", len * KCFG_PAGE_SIZE);
                    started = true;
                    klog('i', "%lx -> %lx", startVirt, startPhy);
                    klog_flush();
                    len = 1;
                } else {
                    len++;
                }

                lastVirt = startVirt;
                lastPhy = startPhy;
            }
            addr += KCFG_PAGE_SIZE;
        } else {
            if (node->entries[i].present) {
                recursiveDump(node_get_child(node, i, false), level + 1);
            } else {
                addr += skips[level] * KCFG_PAGE_SIZE;
            }
        }
    }
}

void Memory::dump(page_tree_node_t* root) {
    recursiveDump(root, 0);
}
