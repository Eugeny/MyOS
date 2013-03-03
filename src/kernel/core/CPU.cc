#include <core/CPU.h>

uint32_t MSR_STAR = 0xC0000081;
uint32_t MSR_LSTAR = 0xC0000082;
uint32_t MSR_FSBASE = 0xC0000100;
uint32_t MSR_GSBASE = 0xC0000101;
 

uint64_t CPU::getCR0() {
    uint64_t v;
    asm volatile("mov %%cr0, %0": "=r"(v));
    return v;    
}

uint64_t CPU::getCR2() {
    uint64_t v;
    asm volatile("mov %%cr2, %0": "=r"(v));
    return v;    
}

uint64_t CPU::getCR3() {
    uint64_t v;
    asm volatile("mov %%cr3, %0": "=r"(v));
    return v;    
}

uint64_t CPU::getCR4() {
    uint64_t v;
    asm volatile("mov %%cr4, %0": "=r"(v));
    return v;    
}

void CPU::setCR0(uint64_t v) {
    asm volatile("mov %0, %%cr0":: "r"(v));
}

void CPU::setCR2(uint64_t v) {
    asm volatile("mov %0, %%cr2":: "r"(v));
}

void CPU::setCR3(uint64_t v) {
    asm volatile("mov %0, %%cr3":: "r"(v));
}

void CPU::setCR4(uint64_t v) {
    asm volatile("mov %0, %%cr4":: "r"(v));
}


void CPU::enableSSE() {
    asm volatile("fninit");
    CPU::setCR0(CPU::getCR0() | (1 << 5));
    CPU::setCR4(CPU::getCR4() | (1 << 10) | (1 << 9));
}

void CPU::invalidateTLB(uint64_t v) {
    asm volatile("invlpg %0" :: "m"(v) : "memory");
}


void CPU::CLI() {
    asm volatile("cli");
}

void CPU::STI() {
    asm volatile("sti");
}

void CPU::CLTS() {
    asm volatile("clts");
}

void CPU::halt() {
    asm volatile("hlt");
}

uint64_t CPU::RDMSR(uint32_t msr_id) {
    uint64_t msr_value;
    //if (msr_id == MSR_FSBASE)
        //asm volatile("rdfsbase %%rax" : "=a" (msr_value));
    //else
        asm volatile("rdmsr" : "=A" (msr_value) : "c" (msr_id));
    return msr_value;
}

void CPU::WRMSR(uint32_t msr_id, uint64_t msr_value) {
    //if (msr_id == MSR_FSBASE)
        //asm volatile("wrfsbase %%rbx" :: "b" (msr_value));
    //else
        asm volatile("wrmsr" :: "c" (msr_id), "A" (msr_value));
}