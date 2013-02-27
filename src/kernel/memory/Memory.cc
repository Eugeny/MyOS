#include <core/CPU.h>
#include <memory/AddressSpace.h>
#include <memory/FrameAlloc.h>
#include <memory/Memory.h>
#include <kutil.h>


// -----------------------------
// Page tree operations


void Memory::init() {
    AddressSpace::kernelSpace->setRoot((page_tree_node_t*) 0x20000);
    AddressSpace::kernelSpace->initEmpty();

    FrameAlloc::get()->init((512*1024*1024) / KCFG_PAGE_SIZE);

    
    for (uint64_t i = 0; i < KCFG_LOW_IDENTITY_PAGING_LENGTH; i += KCFG_PAGE_SIZE) {
        AddressSpace::kernelSpace->mapPage(
            AddressSpace::kernelSpace->getPage(i, true), 
            i, PAGEATTR_SHARED
        );
    }

    AddressSpace::kernelSpace->namePage(
        AddressSpace::kernelSpace->getPage(0, false),
        "Kernel mapping"
    );

    for (uint64_t i = 0; i <= KCFG_HIGH_IDENTITY_PAGING_LENGTH; i += KCFG_PAGE_SIZE) { 
        AddressSpace::kernelSpace->mapPage(
            AddressSpace::kernelSpace->getPage(0xffffffffffffffff - KCFG_HIGH_IDENTITY_PAGING_LENGTH + i, true),
            KCFG_LOW_IDENTITY_PAGING_LENGTH + i, 0
        );
    }

    AddressSpace::kernelSpace->namePage(
        AddressSpace::kernelSpace->getPage(0xffffffffffffffff - KCFG_HIGH_IDENTITY_PAGING_LENGTH, false),
        "Aux mapping"
    );

    AddressSpace::kernelSpace->activate();
}


void Memory::handlePageFault(isrq_registers_t* reg) {
    AddressSpace::current->dump();

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

