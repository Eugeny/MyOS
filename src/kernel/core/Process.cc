#include <core/Process.h>
#include <kutil.h>


Process::Process() {
    brk = 0x400000;
    isKernel = false;
}

void* Process::sbrk(uint64_t size) {
    addressSpace->allocateSpace(brk, size, isKernel ? 0 : PAGEATTR_USER);
    void* result = (void*)brk;
    brk += size;
    return result;
}

Process::~Process() {
}
