#include <elf/ELF.h>
#include <core/TaskManager.h>
#include <memory/AddressSpace.h>
#include <memory/Memory.h>
#include <syscall/Syscalls.h>
#include <kutils.h>

void ELF_exec(u8int* data, int argc, char** argv, FileObject* stdin, FileObject* stdout, FileObject* stderr) {
    int pid = fork();

    if (pid == 0) {
        AddressSpace* as = Memory::get()->getCurrentSpace();
        elfHeader* hdr = (elfHeader*)data;
        for (int i = 0; i < hdr->phnum; i++) {
            programHeader* ph = (programHeader*)(data + hdr->phoff + i * hdr->phentsize);
            if (ph->type == PT_LOAD) {
                DEBUG(to_hex(ph->paddr));
                DEBUG(to_hex(ph->offset));
                for (int j = ph->vaddr; j < ph->vaddr + ph->memSize; j += 0x1000)
                    as->allocatePage(j, true, false, false);
                memcpy((void*)ph->vaddr, (void*)(data + ph->offset), ph->fileSize);
                memset((void*)ph->vaddr + ph->fileSize, 0, ph->memSize - ph->fileSize);
            }
        }

        Process* p = TaskManager::get()->getCurrentThread()->process;
        p->reopenFile(0, stdin);
        p->reopenFile(1, stdout);
        p->reopenFile(2, stderr);
        asm volatile ("    \
            push %2;       \
            push %1;       \
            mov %0, %%eax; \
            jmp *%%eax     \
            " :: "r"(hdr->entry),
                 "r"(argc),
                 "r"(argv));
    }
}
