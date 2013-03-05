#include <memory/AddressSpace.h>

#include <core/Scheduler.h>
#include <memory/FrameAlloc.h>
#include <memory/Memory.h>
#include <alloc/malloc.h>
#include <tty/Escape.h>
#include <core/CPU.h>
#include <kutil.h>
#include <string.h>

static AddressSpace __kernelSpace;

AddressSpace* AddressSpace::kernelSpace = &__kernelSpace;
AddressSpace* AddressSpace::current = NULL;

#define ADDR_TRAP 0xcccccccc


static page_tree_node_t* allocate_node() {
    return (page_tree_node_t*)kvalloc(sizeof(page_tree_node_t));
}

static void initialize_node_entry(page_tree_node_entry_t* entry) {
    entry->present = 0;
    entry->rw = 1;
    entry->user = 1;
    entry->unused = 0;
    entry->address = ADDR_TRAP; // trap
}

static void initialize_node(page_tree_node_t* node) {
    for (int idx = 0; idx < 512; idx++) {
        initialize_node_entry(&(node->entries[idx]));
        node->entriesVirtual[idx] = 0;
        node->entriesAttrs[idx] = 0;
        node->entriesNames[idx] = 0;
    }
}


static page_tree_node_t* node_get_child(page_tree_node_t* node, uint64_t idx, bool create) {
    if (!node->entries[idx].present) {
        if (!create) {
            return NULL;
        }
        page_tree_node_t* child = allocate_node();
        initialize_node(child);
        node->entries[idx].present = 1;
        node->entriesVirtual[idx] = child;
        if (AddressSpace::current)
            node->entries[idx].address = AddressSpace::current->getPhysicalAddress((uint64_t)child) / KCFG_PAGE_SIZE; 
        else
            node->entries[idx].address = (uint64_t)child / KCFG_PAGE_SIZE; 

        return child;
    }
    return node->entriesVirtual[idx];
}


static void copy_page_physical(uint64_t src, uint64_t dst) {
    AddressSpace::current->mapPage(AddressSpace::current->getPage(KCFG_TEMP_PAGE_1, true), src, PAGEATTR_SHARED);
    AddressSpace::current->mapPage(AddressSpace::current->getPage(KCFG_TEMP_PAGE_2, true), dst, PAGEATTR_SHARED);
    memcpy((void*)KCFG_TEMP_PAGE_2, (void*)KCFG_TEMP_PAGE_1, KCFG_PAGE_SIZE);
    //AddressSpace::current->releasePage(AddressSpace::current->getPage(KCFG_TEMP_PAGE_1, false));
    //AddressSpace::current->releasePage(AddressSpace::current->getPage(KCFG_TEMP_PAGE_2, false));
}



// -------------------------------


AddressSpace::AddressSpace() {
}

AddressSpace::~AddressSpace() {
    delete root;
}

void AddressSpace::initEmpty() {
    if (!getRoot())
        setRoot(allocate_node());
    initialize_node(getRoot());
}

void AddressSpace::activate() {
    if (AddressSpace::current) {
        //klog('t',"Switching address space: %016lx",AddressSpace::current->getPhysicalAddress((uint64_t)root));
        CPU::setCR3(AddressSpace::current->getPhysicalAddress((uint64_t)root));
    }
    else
        CPU::setCR3((uint64_t)root);
    current = this;
}

page_tree_node_t* AddressSpace::getRoot() {
    return root;
}

void AddressSpace::setRoot(page_tree_node_t* r) {
    root = r;
}

page_descriptor_t AddressSpace::getPage(uint64_t virt, bool create) {
    page_descriptor_t d;
    page_tree_node_t* root = getRoot();
    d.pageVAddr = virt;

    // Skip memory hole
    uint64_t fixedVirt = virt;

    if (fixedVirt >= 0xffff800000000000) {
        fixedVirt -= 0xffff000000000000;
    }

    uint64_t page = fixedVirt / KCFG_PAGE_SIZE; // page index
    uint64_t indexes[4] = {
        page / 512 / 512 / 512 % 512,
        page / 512 / 512 % 512,
        page / 512 % 512,
        page % 512,
    };

    for (int i = 0; i < 3; i++) {
        root = node_get_child(root, indexes[i], create);

        if (root == NULL && !create) {
            d.entry = 0;
            return d;
        }
    }

    root->entriesVirtual[page % 512] = (page_tree_node_t*)virt;

    d.entry = &(root->entries[page % 512]);
    d.attrs = &(root->entriesAttrs[page % 512]);
    d.vAddr = &(root->entriesVirtual[page % 512]);
    d.name  = &(root->entriesNames[page % 512]);

    return d;
}

uint64_t AddressSpace::getPhysicalAddress(uint64_t virt) {
    return getPage(virt, false).entry->address * KCFG_PAGE_SIZE + (virt % KCFG_PAGE_SIZE);
}

page_descriptor_t AddressSpace::mapPage(page_descriptor_t page, uint64_t phy, uint8_t attrs) {
//    klog('t', "Mapping page: %lx -> %lx", page.pageVAddr, phy);
    FrameAlloc::get()->markAllocated(phy / KCFG_PAGE_SIZE);
    page.entry->present = true;
    page.entry->user = true;
    page.entry->rw = true;
    page.entry->address = phy / KCFG_PAGE_SIZE;
    *(page.attrs) = attrs;
    *(page.vAddr) = (page_tree_node_t*)page.pageVAddr;
    return page;
}

void AddressSpace::namePage(page_descriptor_t page, char* name) {
    *(page.name) = name;
}

page_descriptor_t AddressSpace::allocatePage(page_descriptor_t page, uint8_t attrs) {
    if (!page.entry->present) {
        uint64_t frame = FrameAlloc::get()->allocate();
        mapPage(page, frame * KCFG_PAGE_SIZE, attrs);
    }
    return page;
}

void AddressSpace::allocateSpace(uint64_t base, uint64_t size, uint8_t attrs) {
    uint64_t top = base + size;
    base = base / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    top = (top + KCFG_PAGE_SIZE - 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    size = top - base;
    klog('t', "Allocating %lx bytes at %lx", size, base);
    for (uint64_t v = base; v < base + size; v += KCFG_PAGE_SIZE) {
        allocatePage(getPage(v, true), attrs);
    }
}

void AddressSpace::releasePage(page_descriptor_t page) {
    if (page.entry->present) {
        FrameAlloc::get()->release(page.entry->address);
        initialize_node_entry(page.entry);
    }
}

void AddressSpace::releaseSpace(uint64_t base, uint64_t size) {
    uint64_t top = base + size;
    base = base / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    top = (top + KCFG_PAGE_SIZE - 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
    size = top - base;
    klog('t', "Releasing %lx bytes at %lx", size, base);
    for (uint64_t v = base; v < base + size; v += KCFG_PAGE_SIZE) {
        releasePage(getPage(v, true));
    }
}

AddressSpace* AddressSpace::clone() {
    CPU::CLI();
    Scheduler::get()->pause();

    klog('w', "Cloning address space from %lx", this);klog_flush();
    AddressSpace* result = new AddressSpace();
    result->initEmpty();
        klog('w', "!");klog_flush();

    page_tree_node_t* node = getRoot();

    dump();
    
    for (int i = 0; i < 512; i++) { // PML4s
        //klog('w', "%i", i);
        if (node->entries[i].present) {
            page_tree_node_t* pml4 = node->entriesVirtual[i];
            
            for (int j = 0; j < 512; j++) { // PDPTs
                if (pml4->entries[j].present) {
                    page_tree_node_t* pd = pml4->entriesVirtual[j];

                    for (int l = 0; l < 512; l++) { // PTs
                        if (pd->entries[l].present) {
                            page_tree_node_t* pt = pd->entriesVirtual[l];

                            for (int m = 0; m < 512; m++) { // Pages
                                if (pt->entries[m].present) {
                                    uint64_t addr = i;
                                    addr = addr * 512 + j;
                                    addr = addr * 512 + l;
                                    addr = addr * 512 + m;
                                    addr *= KCFG_PAGE_SIZE;

                                    if (addr >=       800000000000) {
                                        addr += 0xffff000000000000;
                                    }

                                    page_descriptor_t oldPage = getPage(addr, false);
                                    if (!oldPage.entry) {
                                        klog('e', "NULL PAGE @ %lx", addr);
                                        klog_flush();
                                    //    for(;;);
                                    }
                                    if (oldPage.entry->address == ADDR_TRAP) {
                                        klog('e', "TRAP PAGE @ %lx", addr);
                                        klog_flush();
                                    //    for(;;);
                                    }

                                    if (PAGEATTR_IS_SHARED(*oldPage.attrs)) {
                                        page_descriptor_t page = result->getPage(addr, true);

                                        *(page.vAddr) = *oldPage.vAddr;
                                        *(page.entry) = *oldPage.entry;
                                        *(page.attrs) = *oldPage.attrs;
                                        *(page.name) = *oldPage.name;

                                        if (PAGEATTR_IS_COPY(*oldPage.attrs)) {
                                            page.entry->present = false;
                                            result->allocatePage(page, *oldPage.attrs);
                                            copy_page_physical(
                                                oldPage.entry->address * KCFG_PAGE_SIZE,
                                                page.entry->address * KCFG_PAGE_SIZE
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    klog('w', "Cloned address space into %lx", result);klog_flush();
    CPU::STI();

    result->dump();
    
    return result;
}

void AddressSpace::release() {
}



void AddressSpace::recursiveDump(page_tree_node_t* node, int level) {
    static uint64_t skips[4] = {
        512 * 512 * 512,
        512 * 512,
        512
    };

    static uint64_t addr, startPhy, startVirt, lastVirt, lastPhy, len, index[4];
    static bool started = false;

    if (level == 0) {
        addr = 0;
        startPhy = 0;
        startVirt = 0;
        len = 0;
        started = false;
        lastVirt = -1;
    }

    for (int i = 0; i < 512; i++) {
        if (addr >= 0x0000800000000000 && addr < 0xffff800000000000)
            addr += 0xffff000000000000;

        index[level] = i;

        if (level == 3) {
            if (node->entries[i].present) {
                startVirt = addr;
                startPhy = node->entries[i].address * KCFG_PAGE_SIZE;
                if ((startVirt != lastVirt + KCFG_PAGE_SIZE) || (startPhy != lastPhy + KCFG_PAGE_SIZE)) {
                    if (started)
                        klog('i', "            %slength: %016lx", Escape::C_GRAY, len * KCFG_PAGE_SIZE);
                    started = true;

                    uint8_t attrs = node->entriesAttrs[i];
                    klog('i', "%s%016lx -> %016lx %s [%s %s %s] %s", Escape::C_B_GRAY, startVirt, startPhy, 
                        Escape::C_GRAY,
                        PAGEATTR_IS_SHARED(attrs) ? "SHR": "---",
                        PAGEATTR_IS_USER(attrs)   ? "USR": "KRN",
                        PAGEATTR_IS_COPY(attrs)   ? "CPY": "---",
                        node->entriesNames[i] ? node->entriesNames[i] : "---"
                    );
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

void AddressSpace::dump() {
    klog('w', "Dumping address space %016lx", this);
    recursiveDump(root, 0);
}