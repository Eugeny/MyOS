#ifndef HARDWARE_PIT_PIT_H
#define HARDWARE_PIT_PIT_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>
#include <core/MQ.h>


class PIT : public Singleton<PIT> {
public:
    void init();
    void setFrequency(uint32_t freq);
    uint32_t getFrequency();
    uint64_t getTicks();
    uint64_t getTime();
    static Message MSG_TIMER;
private:
    uint32_t frequency;
};

#endif
