#include <memory/Memory.h>
#include <alloc/malloc.h>
#include <core/CPU.h>
#include <core/Debug.h>
#include <core/Scheduler.h>
#include <memory/AddressSpace.h>
#include <memory/FrameAlloc.h>
#include <kutil.h>


Message Memory::MSG_PAGEFAULT("page-fault");
Message Memory::MSG_GPF("gpf");


static void pfISRQ(isrq_registers_t* reg) {
    Memory::MSG_PAGEFAULT.post((void*)reg);
}

static void gpfISRQ(isrq_registers_t* reg) {
    Memory::MSG_GPF.post((void*)reg);
}


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

    for (uint64_t i = 0; i < KCFG_HIGH_IDENTITY_PAGING_LENGTH; i += KCFG_PAGE_SIZE) { 
        AddressSpace::kernelSpace->mapPage(
            AddressSpace::kernelSpace->getPage(0xffffffffffffffff - KCFG_HIGH_IDENTITY_PAGING_LENGTH + i + 1, true),
            KCFG_LOW_IDENTITY_PAGING_LENGTH + i, PAGEATTR_SHARED | PAGEATTR_COPY
        );
    }

    AddressSpace::kernelSpace->namePage(
        AddressSpace::kernelSpace->getPage(0xffffffffffffffff - KCFG_HIGH_IDENTITY_PAGING_LENGTH + 1, false),
        "Aux mapping"
    );

    AddressSpace::kernelSpace->activate();

    Memory::MSG_PAGEFAULT.registerConsumer((MessageConsumer)&Memory::handlePageFault);
    Memory::MSG_GPF.registerConsumer((MessageConsumer)&Memory::handleGPF);
    Interrupts::get()->setHandler(13, gpfISRQ);
    Interrupts::get()->setHandler(14, pfISRQ);
}

void Memory::log() {
    kheap_info_t mem = kmallinfo();
    klog('i', "Kernel heap: %i/%i KB", mem.used_bytes / 1024, mem.total_bytes / 1024);   
    klog('i', "Phy frames:  %i/%i (%i/%i KB)", 
        FrameAlloc::get()->getAllocated(), FrameAlloc::get()->getTotal(),
        FrameAlloc::get()->getAllocated() * 4, FrameAlloc::get()->getTotal() * 4);   
}

void Memory::handlePageFault(isrq_registers_t* regs) {
    AddressSpace::current->dump();

    const char* fPresent  = (regs->err_code & 1) ? "P" : "-";
    const char* fWrite    = (regs->err_code & 2) ? "W" : "-";
    const char* fUser     = (regs->err_code & 4) ? "U" : "-";
    const char* fRW       = (regs->err_code & 8) ? "R" : "-";
    const char* fIFetch   = (regs->err_code & 16) ? "I" : "-";

    log();

    sout("PAGE FAULT");
    klog('e', "PAGE FAULT [%s%s%s%s%s]", fPresent, fWrite, fUser, fRW, fIFetch);
    klog('e', "Faulting address : %lx", CPU::getCR2());
    klog('e', "Faulting code    : %lx", regs->rip);
    //Debug::MSG_DUMP_REGISTERS.post((void*)regs);
    Scheduler::get()->logTask();
    klog_flush();
    for(;;);
}

void Memory::handleGPF(isrq_registers_t* regs) {
    AddressSpace::current->dump();
 
    log();
  
    sout("GENERAL PROTECTION FAULT");
    klog('e', "GENERAL PROTECTION FAULT");
    klog('e', "Faulting code: %lx", regs->rip);
    klog('e', "Errcode      : %lx", regs->err_code);
    //Debug::MSG_DUMP_REGISTERS.post((void*)regs);
    //Scheduler::get()->logTask();
    klog_flush();
    for(;;);
}