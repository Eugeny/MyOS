#include <core/CPU.h>

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
    CPU::setCR4(CPU::getCR4() | (1 << 8) | (1 << 9));
}

void CPU::invalidateTLB(uint64_t v) {
    asm volatile("invlpg %0" :: "m"(v) : "memory");
}

