


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

void heap_free(heap_t *h, u32int addr) {
    int idx = -1;
    for (int i = 0; i < h->index_length; i++)
        if (h->index[i].addr >= addr) {
            idx = i;
            break;
        }
     
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
}

int heap__find_hole(heap_t *h, u8int align, u32int sz) {
    int best;
    u32int bestsz = 0xFFFFFFFF;
    
    for (int i = 0; i < h->index_length; i++) 
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
       // TODO heap__expand(h, sz + (align?0x1000:0));
        best = h->index_length;
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
    for (int i = idx+1; i < h->index_length; i++)
        h->index[i-1] = h->index[i];
    h->index_length--;
}
