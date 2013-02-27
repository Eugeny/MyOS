#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>


class Memory {
public:
    static void init();

    static void handlePageFault(isrq_registers_t* reg);
private:    
};
#endif
