#include <alloc/malloc.h>
#include <memory/Memory.h>
#include <kutil.h>


page_tree_node_t* __initial_pml4;
uint64_t __initial_heap;

static uint64_t __allocate_heap(uint64_t sz) {
    sz = ((sz + 0x0fff) / 0x1000) * 0x1000;
    return (uint64_t)kvalloc(sz);
    uint64_t res = __initial_heap;
    __initial_heap += sz;
    return res;
}


static page_tree_node_t* allocate_node() {
    return (page_tree_node_t*)__allocate_heap(sizeof(page_tree_node_t));
}

static void initialize_node(page_tree_node_t* node) {
    for (int idx = 0; idx < 512; idx++) {
        node->entries[idx].present = 0;
        node->entries[idx].rw = 1;
        node->entries[idx].user = 1;
        node->entries[idx].address = 0xcccccccc; // trap
    }    
}

static page_tree_node_t* node_get_child(page_tree_node_t* node, uint64_t idx) {
    //__outputhex((uint64_t)__initial_heap,60);
    //for (int c = 0; c < 32000; c++);
    //__outputhex((uint64_t)node,70);
    if ((uint64_t)node > 0x30000){
    //for (int c = 0; c < 3200000; c++);
}
    if (!node->entries[idx].present) {
        page_tree_node_t* child = allocate_node();
        initialize_node(child);
        node->entries[idx].present = 1;
        node->entries[idx].address = (uint64_t)child / 0x1000; 
        return child;
    }
    return (page_tree_node_t*)(node->entries[idx].address * 0x1000);
}

static void map_page(page_tree_node_t* root, uint64_t virt, uint64_t phys) {
    __outputhex((uint64_t)virt ,50);

    if (virt >= 0xffff800000000000) {
        virt -= 0xffff000000000000;
    }

    virt = virt / 0x1000; // page index
    uint64_t indexes[5] = {
        virt / 512 / 512 / 512 / 512 % 512,
        virt / 512 / 512 / 512 % 512,
        virt / 512 / 512 % 512,
        virt / 512 % 512,
        virt % 512,
    };
    for (int i = 0; i < 4; i++) {
        root = node_get_child(root, indexes[i]);
    }
    int index = indexes[4];
    root->entries[index].present = 1;
    root->entries[index].rw = 1;
    root->entries[index].user = 1;
    root->entries[index].address = phys / 0x1000;
}

void load_page_tree(page_tree_node_t* root) {
    asm volatile("mov %0, %%cr3":: "r"(root));
    uint64_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void memory_initialize_default_paging() {
    __initial_pml4 = (page_tree_node_t*) 0x20000;
    initialize_node(__initial_pml4);
    __initial_heap = (uint64_t)__initial_pml4 + sizeof(page_tree_node_t);
    
    for (uint64_t i = 0; i < 0x10000000; i += 0x1000) // 16 mb
        map_page(__initial_pml4, i, i);

    //for (uint64_t i = 0; i < 0x10000000; i += 0x1000) // upper 16 mb
      //  map_page(__initial_pml4, 0xffffffffffffffff - 0x10000000 + i, 0x10000000 + i);

    page_tree_node_t* r = __initial_pml4;
    r = node_get_child(r, 0);
    r = node_get_child(r, 0);
    r = node_get_child(r, 0);
    r = node_get_child(r, 2);
    __outputhex(r->entries[265].present, 20);
    //for(;;);
    load_page_tree(__initial_pml4);
}

// 0 0 0 2 265