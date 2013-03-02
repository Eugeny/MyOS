#include <elf/ELF.h>
#include <core/CPU.h>
#include <alloc/malloc.h>
#include <core/Scheduler.h>
#include <memory/AddressSpace.h>
#include <kutil.h>


ELF::ELF() {

}

void ELF::loadFromFile(File* f) {
    data = (uint8_t*)kmalloc(1024*1024);
    f->read(data, 1024*1024);
    f->close();
}

void ELF::loadIntoProcess(Process* p) {
    auto oldAS = AddressSpace::current;
    auto as = p->addressSpace;
    CPU::CLI();
    as->activate();

    auto hdr = (elf_header_t*)data;
    for (int i = 0; i < hdr->e_phnum; i++) {
        auto ph = (elf_program_header_t*)(data + hdr->e_phoff + i * hdr->e_phentsize);
        if (ph->p_type == PT_LOAD) {
            klog('t', "ELF PT_LOAD %lx+%lx(%lx) -> %lx", ph->p_offset, ph->p_filesz, ph->p_memsz, ph->p_vaddr);
            as->allocateSpace(ph->p_vaddr, ph->p_memsz+0x2000, PAGEATTR_SHARED|PAGEATTR_USER);
            memcpy((void*)ph->p_vaddr, (void*)(data + ph->p_offset), ph->p_filesz);
            memset((void*)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    oldAS->activate();
    CPU::STI();

}

uint64_t ELF::getEntryPoint() {
    auto hdr = (elf_header_t*)data;
    return hdr->e_entry;
}
