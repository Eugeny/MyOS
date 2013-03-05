#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>
#include <string.h>


Process::Process(const char* name) {
    static int last_pid = 0;
    pid = (last_pid++);
    ppid = 0;

    brk = 0x400000;
    stackbrk = 0x70000000000 - 0x2000;
    isKernel = false;
    pty = NULL;
    strcpy(this->name, name);
    strcpy(cwd, "/");
}

Process* Process::clone() {
    Process* p = new Process(*this);
    p->threads.clear();
    return p;
}

void* Process::sbrk(uint64_t size) {
    addressSpace->allocateSpace(brk, size, 
        PAGEATTR_SHARED | (isKernel ? 0 : (PAGEATTR_USER | PAGEATTR_COPY)));
    void* result = (void*)brk;
    brk += size;
    return result;
}

void* Process::sbrkStack(uint64_t size) {
    void* result = (void*)stackbrk;
    stackbrk -= size;
    addressSpace->allocateSpace(stackbrk, size, isKernel ? 0 : (PAGEATTR_SHARED | PAGEATTR_USER));
    addressSpace->namePage(addressSpace->getPage(stackbrk, false), "Stacks");
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
    t->createStack(0x12000);
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
}
