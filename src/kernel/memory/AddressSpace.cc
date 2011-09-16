#include <memory/AddressSpace.h>
#include <memory/Heap.h>
#include <memory/Memory.h>
#include <kutils.h>


extern "C" void copy_page_physical(u32int src, u32int dst);

AddressSpace::AddressSpace() {
    reset();
}

void AddressSpace::reset() {
    u32int phys;
    dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(dir, 0, sizeof(page_directory_t));
    u32int offset = (u32int)dir->tablesPhysical - (u32int)dir;
    dir->physicalAddr = phys + offset;
}

AddressSpace::~AddressSpace() {
    delete dir;
}

page_t *AddressSpace::getPage(u32int address, bool make) {
    address /= 0x1000;
    u32int table_idx = address / 1024;

    if (dir->tables[table_idx])
       return &dir->tables[table_idx]->pages[address%1024];

    if (make) {
        u32int tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        return &dir->tables[table_idx]->pages[address%1024];
    }

    return 0;
}

page_t *AddressSpace::allocatePage(u32int address, bool make, bool kernel, bool rw) {
    page_t* r = getPage(address, make);
    Memory::get()->allocate(r, kernel, rw);
    return r;
}

static page_table_t *clone_table(page_table_t *src, u32int *physAddr) {
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);
    memset(table, 0, sizeof(page_directory_t));

    for (int i = 0; i < 1024; i++) {
        if (src->pages[i].frame) {
            Memory::get()->allocate(&table->pages[i], false, false);
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
        }
    }

    return table;
}

AddressSpace* AddressSpace::clone() {
    AddressSpace* n = new AddressSpace();
    for (int i = 0; i < 1024; i++) {
        if (!dir->tables[i])
            continue;

        if (Memory::get()->getKernelSpace()->dir->tables[i] == dir->tables[i]) {
            n->dir->tables[i] = dir->tables[i];
            n->dir->tablesPhysical[i] = dir->tablesPhysical[i];
        } else {
            u32int phys;
            n->dir->tables[i] = clone_table(dir->tables[i], &phys);
            n->dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return n;
}

void AddressSpace::release() {
    for (int i = 0; i < 1024; i++) {
        if (!dir->tables[i])
            continue;

        if (Memory::get()->getKernelSpace()->dir->tables[i] != dir->tables[i]) {
            for (int j = 0; j < 1024; j++) {
                if (dir->tables[i]->pages[j].frame) {
                    Memory::get()->free(&dir->tables[i]->pages[j]);
                }
            }
        }
    }
}
