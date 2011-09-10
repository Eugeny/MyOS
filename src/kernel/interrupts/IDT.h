#ifndef INTERRUPTS_IDT_H
#define INTERRUPTS_IDT_H

#include <util/Singleton.h>
#include <util/cpp.h>

class IDT : public Singleton<IDT> {
public:
    void init();
    void setGate(u8int, u32int, u16int, u8int);
    void flush();
};
#endif

