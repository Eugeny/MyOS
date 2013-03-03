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
    t->setEntryArguments(argc, (uint64_t)argv, (uint64_t)env, 0, 0, 0);
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

Process::~Process() {
    delete name;
}
