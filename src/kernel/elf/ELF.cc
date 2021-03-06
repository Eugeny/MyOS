#include <elf/ELF.h>
#include <core/CPU.h>
#include <alloc/malloc.h>
#include <core/Scheduler.h>
#include <fs/vfs/VFS.h>
#include <fs/File.h>
#include <memory/AddressSpace.h>
#include <kutil.h>
#include <elf.h>
#include <stdlib.h>
#include <fcntl.h>


ELF::ELF() {
    data = (uint8_t*)kmalloc(4*1024*1024);
}

ELF::~ELF() {
    delete data;
}

void ELF::loadFromFile(char* path) {
    strcpy(exeName, path);
    StreamFile* f = VFS::get()->open(path, O_RDONLY);
    if (f) {
        f->read(data, 4*1024*1024);
        f->close();
    } else {
        klog('t', "ELF file %s not found!", path);
    }
}

void ELF::loadIntoProcess(Process* p) {
    strcpy(p->exeName, exeName);

    auto oldAS = AddressSpace::current;
    auto as = p->addressSpace;
    as->activate();

    auto hdr = (Elf64_Ehdr*)data;
    for (int i = 0; i < hdr->e_phnum; i++) {
        auto ph = (Elf64_Phdr*)(data + hdr->e_phoff + i * hdr->e_phentsize);
        if (ph->p_type == PT_LOAD) {
            klog('t', "ELF PT_LOAD %lx+%lx(%lx) -> %lx", ph->p_offset, ph->p_filesz, ph->p_memsz, ph->p_vaddr);
            as->allocateSpace(ph->p_vaddr, ph->p_memsz+0x2000, PAGEATTR_SHARED|PAGEATTR_USER|PAGEATTR_COPY);
            as->namePage(as->getPage(ph->p_vaddr, false), "ELF Code");

            if (p->brk < ph->p_vaddr + ph->p_memsz)
                p->brk = ph->p_vaddr + ph->p_memsz;

            memcpy((void*)ph->p_vaddr, (void*)(data + ph->p_offset), ph->p_filesz);
            memset((void*)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }
    }

    oldAS->activate();
}

uint64_t ELF::getEntryPoint() {
    auto hdr = (Elf64_Ehdr*)data;
    return hdr->e_entry;
}

Thread* ELF::startMainThread(Process* p, char** argv, char** envp) {
    static uint8_t randomVec[16];
    for (int i = 0; i < 16; i++)
        randomVec[i] = random() % 256;

    Thread* t = p->spawnThread((threadEntryPoint)getEntryPoint(), "main");
    
    #define addAuxVector(a, v) { t->pushOnStack((uint64_t)v); t->pushOnStack(a); }
    #define stackify(s) ((char*)t->pushOnStack(s, strlen(s) + 1))

    t->pushOnStack(0);
    addAuxVector(AT_NULL, 0);
    addAuxVector(AT_RANDOM, randomVec);
    addAuxVector(AT_PLATFORM, "x86_64");
    addAuxVector(AT_PAGESZ, KCFG_PAGE_SIZE);
    addAuxVector(AT_ENTRY, getEntryPoint());
    addAuxVector(AT_UID,  0);
    addAuxVector(AT_GID,  0);
    addAuxVector(AT_EUID, 0);
    addAuxVector(AT_EGID, 0);

    char* new_argv[256];
    char* new_envp[256];

    int envpc = 0;
    if (envp) {
        klog('d', "ELF Environment:");
        while (envp[envpc] != NULL) {
            klog('d', envp[envpc]);
            new_envp[envpc] = stackify(envp[envpc]);
            envpc++;
        }
    }
    
    t->pushOnStack(0);
    for (int i = envpc - 1; i >= 0; i--)
        t->pushOnStack((uint64_t)new_envp[i]);
    
    uint64_t p_env = t->state.regs.rsp;

    int argc = 0;
    klog('d', "ELF Arguments:");
    while (argv[argc] != NULL) {
        klog('d', "%s = 0x%lx", argv[argc], argv[argc]);
        new_argv[argc] = stackify(argv[argc]);
        argc++;
    }
    klog('d', "(%i total)", argc);
    
    t->pushOnStack(0);
    for (int i = argc - 1; i >= 0; i--)
        t->pushOnStack((uint64_t)new_argv[i]);
    
    uint64_t p_argv = t->state.regs.rsp;
    
    t->pushOnStack(argc);

    t->setEntryArguments((uint64_t)argc, (uint64_t)p_argv, (uint64_t)p_env, 0, 0, 0);
    return t;
}
