#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>
#include <core/MQ.h>


class Memory {
public:
    static void init();
    static void handlePageFault(isrq_registers_t* reg);
    static void handleGPF(isrq_registers_t* reg);
    static void log();
    static Message MSG_PAGEFAULT;
    static Message MSG_GPF;
private:    
};
#endif
