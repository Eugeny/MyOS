#include "malloc.h"
#include "lang/lang.h"
#include "memory/AddressSpace.h"
#include "kutil.h"

// -----------------------------
// DMALLOC BEGIN
// -----------------------------

    #define USE_DL_PREFIX
    #define MORECORE __dlmalloc_sbrk
    //#define MORECORE_CANNOT_TRIM 1
    #define MORECORE_CONTIGUOUS 0
    #define ABORT __dlmalloc_abort
    #define MALLOC_FAILURE_ACTION __dlmalloc_fail
    #define MALLINFO_FIELD_TYPE uint32_t
    #define ENOMEM 0
    #define EINVAL 0
    #define HAVE_MMAP 0

    #define LACKS_UNISTD_H
    //#define LACKS_FCNTL_H
    #define LACKS_SYS_PARAM_H
    #define LACKS_SYS_MMAN_H
    #define LACKS_STRINGS_H
    //#define LACKS_STRING_H
    //#define LACKS_SYS_TYPES_H
    #define LACKS_ERRNO_H
    #define LACKS_STDLIB_H
    #define LACKS_SCHED_H
    #define LACKS_TIME_H

    typedef int64_t ptrdiff_t;

    static char theap[KCFG_TEMPHEAP_SIZE];
    static void* hptr = (void*)theap;
    static bool large_heap_active = false;

    extern "C" void* __dlmalloc_sbrk(int size) {
        void* r = hptr;
        hptr = (void*)((uint64_t)hptr + size);
        klog('t', "ksbrk(%lx) = %lx", size, r);
        if (hptr > theap + KCFG_TEMPHEAP_SIZE && !large_heap_active) {
            klog('e', "Temporary heap overflow");
            klog_flush();
            for(;;);
        }
        if ((uint64_t)hptr > KCFG_KERNEL_HEAP_START + KCFG_KERNEL_HEAP_SIZE) {
            klog('e', "Kernel heap overflow");
            klog_flush();
            for(;;);   
        }
        return r;
    }

    extern "C" void __dlmalloc_abort() {
        KTRACE
        klog('e', "dmalloc abort");
        klog_flush();
        for(;;);
    }

    extern "C" void __dlmalloc_fail() {
        KTRACE
        klog('e', "dmalloc fail");
        klog_flush();
        for(;;);
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    #pragma GCC diagnostic ignored "-Waddress"
    #include "_dlmalloc.c"
    #pragma GCC diagnostic pop

// -----------------------------
// DMALLOC END
// -----------------------------

void kalloc_switch_to_main_heap() {
    klog('t', "Enabling main heap");
    AddressSpace::kernelSpace->allocateSpace(KCFG_KERNEL_HEAP_START, KCFG_KERNEL_HEAP_SIZE_INITIAL, PAGEATTR_SHARED);

    AddressSpace::kernelSpace->namePage(
        AddressSpace::kernelSpace->getPage(KCFG_KERNEL_HEAP_START, false),
        "Kernel heap"
    );

    hptr = (void*)KCFG_KERNEL_HEAP_START;
    large_heap_active = true;

    AddressSpace::kernelSpace->allocateSpace(KCFG_KERNEL_HEAP_START, KCFG_KERNEL_HEAP_SIZE, PAGEATTR_SHARED);
}

void* kmalloc(int size) {
    return dlmalloc(size);
}

void* kvalloc(int size) {
    return dlmemalign(KCFG_PAGE_SIZE, size);
    void* ptr = dlmalloc(size + KCFG_PAGE_SIZE);
    ptr = (void*)(((uint64_t)ptr + KCFG_PAGE_SIZE - 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE);
    return ptr;
}

void kmalloc_trim() {
//    dlmalloc_trim(0);
}



void  kfree(void* ptr) {
    dlfree(ptr);
}


kheap_info_t kmallinfo() {
    kheap_info_t info;
    //mallinfo mi = dlmallinfo();
    //info.total_bytes = mi.arena;
    //info.used_bytes =  mi.uordblks;
    info.total_bytes = KCFG_KERNEL_HEAP_SIZE;
    info.used_bytes =  (uint64_t)hptr - KCFG_KERNEL_HEAP_START;
    return info;
}