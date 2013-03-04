#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>
#include <string.h>


Process::Process(const char* name) {
    brk = 0x400000;
    isKernel = false;
    pty = NULL;
    this->name = strdup(name);
}

void* Process::sbrk(uint64_t size) {
    addressSpace->allocateSpace(brk, size, isKernel ? 0 : PAGEATTR_USER);
    void* result = (void*)brk;
    brk += size;
    return result;
}

Thread* Process::spawnThread(threadEntryPoint entry, const char* name) {
    Thread* t = new Thread(this, name);
    t->state = Scheduler::get()->kernelThread->state;
    for (Thread* thread : threads) {
        t->state = thread->state;
        break;
    }
    t->state.addressSpace = addressSpace;
    t->createStack(0x2000);
    t->pushOnStack(0);
    t->pushOnStack(0);
    t->state.regs.rip = (uint64_t)entry;
    threads.add(t);
    Scheduler::get()->registerThread(t);
    return t;
}

Thread* Process::spawnMainThread(threadEntryPoint entry) {
    Thread* t = spawnThread(entry, "main");
    t->setEntryArguments(argc, (uint64_t)argv, (uint64_t)env, (uint64_t)auxv, 0, 0);

    t->pushOnStack(0);
    t->pushOnStack(0); // AT_NULL
    for (int i = auxvc-1; i >= 0; i--) {
        t->pushOnStack(auxv[i].a_un.a_val);
        t->pushOnStack(auxv[i].a_type);
    }
    t->pushOnStack(0);
    for (int i = envc-1; i >= 0; i--)
        t->pushOnStack((uint64_t)env[i]);
    t->pushOnStack(0);
    for (int i = argc-1; i >= 0; i--)
        t->pushOnStack((uint64_t)argv[i]);
    t->pushOnStack(argc);

    return t;
}

int Process::attachFile(File* f) {
    return files.add(f);
}

void Process::closeFile(int fd) {
    File* f = files[fd];
    files.remove(f);
    f->close();
    delete f;
}

void Process::setAuxVector(int idx, uint64_t type, uint64_t val) {
    auxv[idx].a_type = type;
    auxv[idx].a_un.a_val = val;
}

Process::~Process() {
    delete name;
}
