#ifndef HARDWARE_PIT_PIT_H
#define HARDWARE_PIT_PIT_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>


class PIT : public Singleton<PIT> {
public:
    void init();
    void setFrequency(uint32_t freq);
    static const char* MSG_TIMER;
};

#endif
