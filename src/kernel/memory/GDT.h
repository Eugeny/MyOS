#ifndef MEMORY_GDT_H
#define MEMORY_GDT_H

#include <util/cpp.h>
#include <util/Singleton.h>

class GDT : public Singleton<GDT> {
public:
    void init();
    void setDefaults();
    void setGate(s32int num, u32int base, u32int limit, u8int access, u8int gran);
    void flush();
};

#endif
