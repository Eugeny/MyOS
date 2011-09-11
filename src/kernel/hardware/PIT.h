#ifndef HARDWARE_PIT_H
#define HARDWARE_PIT_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <interrupts/Interrupts.h>

class PIT : public Singleton<PIT> {
public:
    void setFrequency(u32int freq);
    void setHandler(interrupt_handler h);
};

#endif
