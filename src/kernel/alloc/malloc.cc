#include "kutil.h"

// -----------------------------
// DMALLOC BEGIN
// -----------------------------

    #define USE_DL_PREFIX
    #define MORECORE dlmalloc_sbrk
    #define ABORT dlmalloc_abort
    #define MALLOC_FAILURE_ACTION dlmalloc_fail
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

    typedef int ptrdiff_t;

    char heap[10240];
    void* hptr = (void*)heap;

    extern "C" void* dlmalloc_sbrk(int size) {
        KTRACE
        void* r = hptr;
        hptr = (void*)((int)hptr + size);
        return r;
    }

    extern "C" void dlmalloc_abort() {
        KTRACE
    }

    extern "C" void dlmalloc_fail() {
        KTRACE
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    #pragma GCC diagnostic ignored "-Waddress"
    #include "_dlmalloc.c"
    #pragma GCC diagnostic pop

// -----------------------------
// DMALLOC END
// -----------------------------


extern "C" void* kmalloc(int size) {
    KTRACE
    return dlmalloc(size);
}
