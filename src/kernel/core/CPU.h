#ifndef CORE_CPU_H
#define CORE_CPU_H

#include <lang/lang.h>
#include <lang/Singleton.h>


extern "C" uint32_t MSR_STAR;
extern "C" uint32_t MSR_LSTAR;
extern "C" uint32_t MSR_FSBASE;
extern "C" uint32_t MSR_GSBASE; 


class CPU {
public:
    static uint64_t getCR0();
    static uint64_t getCR2();
    static uint64_t getCR3();
    static uint64_t getCR4();
    static void     setCR0(uint64_t);
    static void     setCR2(uint64_t);
    static void     setCR3(uint64_t);
    static void     setCR4(uint64_t);

    static void     CLI();
    static void     STI();
    static void     CLTS();
    static void     halt();
    static void     enableSSE();
    static void     invalidateTLB(uint64_t);

    static uint64_t RDMSR(uint32_t msr_id);
    static void     WRMSR(uint32_t msr_id, uint64_t msr_value);
};

#endif
