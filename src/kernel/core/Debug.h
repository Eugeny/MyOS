#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H

#include <lang/lang.h>
#include <interrupts/Interrupts.h>


class Debug {
public:
    static const char* MSG_DUMP_REGISTERS;
    static void init();
private:
    static void onDumpRegisters(isrq_registers_t*);
};

#endif
