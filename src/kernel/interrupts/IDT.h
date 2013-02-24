#ifndef INTERRUPTS_IDT_H
#define INTERRUPTS_IDT_H

#include <lang/lang.h>
#include <lang/Singleton.h>


class IDT : public Singleton<IDT> {
public:
    void init();
    void setGate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
    void flush();
};

#endif
