#include <elf/ELF.h>
#include <core/TaskManager.h>
#include <memory/AddressSpace.h>
#include <memory/Memory.h>
#include <kutils.h>


typedef int(*mainf)(int,char**);

void ELF_exec(u8int* data, char* pname, int argc, char** argv, FileObject* stdin, FileObject* stdout, FileObject* stderr) {
    int pid = TaskManager::get()->fork();

    if (pid == 0) {
        AddressSpace* as = Memory::get()->getCurrentSpace();
        elfHeader* hdr = (elfHeader*)data;
        for (int i = 0; i < hdr->phnum; i++) {
            programHeader* ph = (programHeader*)(data + hdr->phoff + i * hdr->phentsize);
            if (ph->type == PT_LOAD) {
                for (int j = ph->vaddr; j < ph->vaddr + ph->memSize; j += 0x1000)
                    as->allocatePage(j, true, false, false);
                DEBUG("COPY");
                DEBUG(to_hex(ph->vaddr));
                //DEBUG(to_hex((u32int)(data+ph->offset)));
                //DEBUG(to_hex(ph->fileSize));
                memcpy((void*)ph->vaddr, (void*)(data + ph->offset), ph->fileSize);
                //memcpy((void*)(data + ph->offset), (void*)ph->vaddr, ph->fileSize);
                //memdump((void*)(data + ph->offset));
                //memdump((void*)ph->vaddr);
                memset((void*)ph->vaddr + ph->fileSize, 0, ph->memSize - ph->fileSize);
            }
        }
        //memdump((void*)(data));
        //memdump((void*)hdr->entry);
        Process* p = TaskManager::get()->getCurrentThread()->process;
        p->name = strdup(pname);
        p->reopenFile(0, stdin);
        p->reopenFile(1, stdout);
        p->reopenFile(2, stderr);
        TRACE
        mainf main = (mainf)hdr->entry;
        TRACE
        DEBUG(to_hex((u32int)main));
        main(argc, argv);
        TRACE
        for(;;);
//        kprint("out");


        TaskManager::get()->requestKillProcess(TaskManager::get()->getCurrentThread()->id);
    }
}
