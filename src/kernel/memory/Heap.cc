#include <memory/Heap.h>
#include <memory/Memory.h>
#include <kutils.h>

// Generic heap code

int          heap__find_hole(heap_t *h, u8int align, u32int sz);
heap_item_t *heap__insert(heap_t *h, int idx);
void         heap__remove(heap_t *h, int idx);

void heap_init(heap_t *h, u32int indexsz) {
    h->index = (heap_item_t*)kmalloc(indexsz * sizeof(heap_item_t));
    h->index_length = 0;
    heap_item_t *i = heap__insert(h, 0);
    i->addr  = h->base;
    i->size  = h->size;
    i->flags = HEAP_HOLE;
}

void *heap_alloc(heap_t *h, u8int align, u32int sz) {
    int idx = heap__find_hole(h, align, sz);

    heap_item_t *hole = &(h->index[idx]);
    heap_item_t *allc = 0;

    u32int a = hole->addr;
    u32int e = hole->size + a;
    if ((a & 0xFFF) && align) {
        a &= 0xFFFFF000;
        a += 0x1000;
    }

    if (a > hole->addr) {
        hole->size = a - hole->addr;
        allc = heap__insert(h, idx+1);
        allc->addr = a;
        idx++;
    } else {
        allc = hole;
    }

    allc->size = sz;
    allc->flags = 0;

    if (a + sz < e) {
        hole = heap__insert(h, idx+1);
        hole->addr = a + sz;
        hole->size = e - a - sz;
        hole->flags = HEAP_HOLE;
    }

    return (void*)a;
}

u32int heap_free(heap_t *h, u32int addr) {
    u32int idx = -1, res = 0;
    for (u32int i = 0; i < h->index_length; i++)
        if (h->index[i].addr >= addr) {
            idx = i;
            break;
        }
if (h->index[idx].addr > addr) {TRACE}
    res = h->index[idx].size;

    if (idx < h->index_length-1)
        if (h->index[idx+1].flags & HEAP_HOLE) {
            h->index[idx].flags = 0;
            h->index[idx].size += h->index[idx+1].size;
            heap__remove(h, idx+1);
        }

    if (idx > 0)
        if (h->index[idx-1].flags & HEAP_HOLE) {
            h->index[idx-1].size += h->index[idx].size;
            heap__remove(h, idx--);
        }

    h->index[idx].flags = HEAP_HOLE;
    return res;
}

int heap__find_hole(heap_t *h, u8int align, u32int sz) {
    u32int best;
    u32int bestsz = 0xFFFFFFFF;

    for (u32int i = 0; i < h->index_length; i++)
        if ((h->index[i].flags & HEAP_HOLE) &&
            (h->index[i].size >= sz)) {
            u32int a = h->index[i].addr;
            u32int e = h->index[i].size + a;
            if ((a & 0xFFF) && align) {
                a &= 0xFFFFF000;
                a += 0x1000;
            }
            if (((e-a) >= sz) && ((e-a) < bestsz)) {
                bestsz = (e-a);
                best = i;

                if (sz == bestsz) break;
            }
        }

    if (bestsz == 0xFFFFFFFF) {
        PANIC("Out of kernel heap");
    }
    return best;
}

heap_item_t *heap__insert(heap_t *h, int idx) {
    for (int i = h->index_length; i > idx; i--)
        h->index[i] = h->index[i-1];
    h->index_length++;
    return &(h->index[idx]);
}

void heap__remove(heap_t *h, int idx) {
    for (u32int i = idx+1; i < h->index_length; i++)
        h->index[i-1] = h->index[i];
    h->index_length--;
}

// End of generic heap code


#define KHEAP_BASE 0xC0000000
#define KHEAP_INDEX_SIZE 0x200000
#define KHEAP_SIZE 0x4000000

extern u32int end; // Linker provided, end of kernel image
static u32int free_space_start = (u32int)&end;
static u32int usage = 0;


void heap_selfinit() {
    static Heap heap;
    Heap::_selfinit(&heap);
}

void Heap::_selfinit(Heap *h) {
    Heap::_instance = h;
}

void Heap::init() {
    heap_ready = false;
    kheap.base = KHEAP_BASE;
    kheap.size = KHEAP_SIZE;
    heap_init(&kheap, KHEAP_INDEX_SIZE);
}


void* Heap::malloc(u32int sz) {
    return this->malloc(sz, false, NULL);
}

void* Heap::malloc(u32int sz, bool align, u32int *phys) {
    if (!heap_ready)
        return (void*)malloc_dumb(sz, align, phys);
    else
        return (void*)malloc_kheap(sz, align, phys);
}

void Heap::free(void* addr) {
    if (heap_ready) {
        usage -= heap_free(&kheap, (u32int)addr);
    }
}


u32int Heap::getFreeSpaceBoundary() {
    return free_space_start;
}

u32int Heap::malloc_dumb(u32int sz, u8int align, u32int *phys) {
    u32int addr = free_space_start;

    if (align == 1 && (addr & 0xFFF)) {
        addr &= 0xFFFFF000;
        addr += 0x1000;
    }

    if (phys)
        *phys = addr;

    u32int tmp = addr;
    addr += sz;
    free_space_start = addr;
    return tmp;
}

u32int Heap::malloc_kheap(u32int sz, u8int align, u32int *phys) {
    u32int addr = (u32int)heap_alloc(&kheap, align, sz);
    if (phys != 0) {
        page_t *page = Memory::get()->getKernelSpace()->getPage((u32int)addr, false);
        *phys = page->frame*0x1000 + (((u32int)addr)&0xFFF);
    }
    usage += sz;
    return addr;
}


// Identity-maps the kernel memory and enables higher heap
void Heap::switchToHeap() {
    for (u32int i = kheap.base; i < kheap.base + kheap.size; i += 0x1000)
        Memory::get()->getKernelSpace()->getPage(i, true);

    u32int frame = 0;
    while (frame < free_space_start)
    {
        Memory::get()->getKernelSpace()->allocatePage(frame, true, false, false);
        frame += 0x1000;
    }

    for (u32int i = kheap.base; i < kheap.base + kheap.size; i += 0x1000)
        Memory::get()->getKernelSpace()->allocatePage(i, false, false, true);

    heap_ready = true;
}

u32int Heap::getUsage() {
    return usage;
}

// Conventional functions

void* kmalloc_a(u32int sz) {
    return Heap::get()->malloc(sz, true, NULL);
}

void* kmalloc_p(u32int sz, u32int *phys) {
    return Heap::get()->malloc(sz, false, phys);
}

void* kmalloc_ap(u32int sz, u32int *phys) {
    return Heap::get()->malloc(sz, true, phys);
}

void* kmalloc(u32int sz) {
    return Heap::get()->malloc(sz, false, NULL);
}

void  kfree(void *addr) {
    Heap::get()->free(addr);
}
