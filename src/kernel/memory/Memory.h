#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <util/Singleton.h>
#include <util/cpp.h>
#include <memory/AddressSpace.h>

class Memory : public Singleton<Memory> {
public:
    void          startPaging();
    AddressSpace *getKernelSpace();
    AddressSpace *getCurrentSpace();
    u32int        getTotalFrames();
    u32int        getUsedFrames();
    void          allocate(page_t* page, bool kernel, bool rw);
    void          free(page_t* page);
    void          setAddressSpace(AddressSpace *s);
    void          switchAddressSpace(AddressSpace *s);
//private:
    AddressSpace *kernelSpace, *currentSpace;
};

#endif
