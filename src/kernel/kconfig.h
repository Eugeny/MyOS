//#define KCFG_LOG_FORCE_RENDER

#define KCFG_ENABLE_TRACING
#define KCFG_ENABLE_MEMTRACING
#define KCFG_STRACE
//#define KCFG_STRACE2

#define KCFG_TEMPHEAP_SIZE 819200

#define KCFG_PAGE_SIZE 0x1000
#define KCFG_PML4_LOCATION 0x10000
#define KCFG_LOW_IDENTITY_PAGING_LENGTH 0x400000
#define KCFG_HIGH_IDENTITY_PAGING_LENGTH 0x10000
#define KCFG_PAGING_POOL_START (KCFG_LOW_IDENTITY_PAGING_LENGTH + KCFG_HIGH_IDENTITY_PAGING_LENGTH)
#define KCFG_KERNEL_HEAP_START 0xfffffffff0000000
#define KCFG_KERNEL_HEAP_SIZE  0x0000000005000000
#define KCFG_KERNEL_HEAP_SIZE_INITIAL  0x0000000001000000

#define KCFG_TEMP_PAGE_1 0xffffffffe0000000
#define KCFG_TEMP_PAGE_2 0xffffffffe0001000

/* 
MEMORY MAP

0                   -   0x1000000           Identity map
0x100000            -   0x3ffffff           Kernel

0xffffffffe0000000                          Temp page 1
0xffffffffe0001000                          Temp page 2
0xfffffffff0000000  -   0xfffffffff1000000  Kernel heap
TOP-0x1000        -   TOP                   Aux map


*/