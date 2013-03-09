#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H

#include <lang/lang.h>
#include <interrupts/Interrupts.h>
#include <core/MQ.h>


class Debug {
public:
    static Message MSG_DUMP_REGISTERS;
    static Message MSG_DUMP_TASKS;
    static Message MSG_DUMP_ADDRESS_SPACE;
    static void init();
    static bool tracingOn;
private:
    static void onDumpRegisters(isrq_registers_t*);
    static void onDumpTasks(void*);
    static void onDumpAddressSpace(void*);
};

#endif
