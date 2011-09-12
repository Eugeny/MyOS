#ifndef CORE_PROCESSOR_H
#define CORE_PROCESSOR_H

#include <util/Singleton.h>
#include <util/cpp.h>


class Processor : public Singleton<Processor> {
public:
    static u32int getInstructionPointer();
    static u32int getStackPointer();
    static void   disableInterrupts();
    static void   enableInterrupts();
    static void   idle();
};

#endif
