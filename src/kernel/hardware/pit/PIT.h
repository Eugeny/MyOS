#ifndef HARDWARE_PIT_PIT_H
#define HARDWARE_PIT_PIT_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>


class PIT : public Singleton<PIT> {
public:
    void setFrequency(uint32_t freq);
    void setHandler(interrupt_handler h);
};

#endif
