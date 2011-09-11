#include <util/cpp.h>
#include <memory/Memory.h>
#include <memory/Heap.h>
#include <interrupts/Interrupts.h>
#include "kutils.h"


void Memory::setAddressSpace(AddressSpace *s) {
    currentSpace = s;
}


void Memory::switchAddressSpace(AddressSpace *s) {
    currentSpace = s;
    asm volatile("mov %0, %%cr3":: "r"(s->dir->physicalAddr));
    u32int cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}


u32int Memory::getTotalFrames() {
    return nframes;
}

u32int Memory::getUsedFrames() {
    return used_frames;
}

AddressSpace* Memory::getKernelSpace() {
    return kernelSpace;
}

AddressSpace* Memory::getCurrentSpace() {
    return currentSpace;
}
