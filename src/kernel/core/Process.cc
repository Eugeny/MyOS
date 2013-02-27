#include <core/Process.h>
#include <kutil.h>


Process::Process() {
    brk = 0x400000;
}

void* Process::sbrk(uint64_t size) {
    addressSpace->allocateSpace(brk, size, 0);
    void* result = (void*)brk;
    brk += size;
    return result;
}

Process::~Process() {
}
