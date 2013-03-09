#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>
#include <string.h>


static int makepid() {
    static int last_pid = 0;
    return last_pid++;
}

Process::Process(Process* parent, const char* name) {
    pid = makepid();
    this->parent = parent;
    ppid = parent ? parent->pid : 0;
    pgid = parent ? parent->pgid : pid;

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
    p->pid = makepid();
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
    allocateStack(stackbrk, size);
    return result;
}

void Process::allocateStack(uint64_t base, uint64_t size) {
    klog('t', "Allocating stack at %lx (%lx b)", base ,size);
    addressSpace->allocateSpace(base, size, isKernel ? 0 : (PAGEATTR_USER));
    addressSpace->namePage(addressSpace->getPage(base, false), "Stacks");
}


void Process::requestKill() {
    Scheduler::get()->requestKill(this);
}

void Process::setSignalHandler(int signal, struct sigaction* a) {
    if (signalHandlers[signal])
        delete signalHandlers[signal];
    struct sigaction* na = new struct sigaction();
    *na = *a;
    klog('d', "Installing signal handler %i (%lx/%lx)", signal, a->sa_handler, a->sa_sigaction);
    signalHandlers[signal] = na;
}

void Process::queueSignal(int signal) {
    pendingSignals |= (1 << signal);
}

typedef void (*sa_handler_t)(int);

void Process::runPendingSignals() {
    for (int i = 0; i < 63; i++) {
        if (pendingSignals & (1 << i)) {
            if (signalHandlers[i]) {
                sa_handler_t handler = signalHandlers[i]->sa_handler;
                if (handler == SIG_DFL) ;
                else if (handler == SIG_IGN) ;
                else 
                    handler(i);
            }
        }
    }
    pendingSignals = 0;
}


Thread* Process::spawnThread(threadEntryPoint entry, const char* name) {
    Thread* t = new Thread(this, name);
    t->state = Scheduler::get()->kernelThread->state;
    for (Thread* thread : threads) {
        t->state = thread->state;
        break;
    }
    t->state.addressSpace = addressSpace;
    t->createStack(0x900000);
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

void Process::realpath(char* p, char* buf) {
    strcpy(buf, "");
    if (p[0] != '/') {
        if (strlen(cwd) > 1)
            strcpy(buf, cwd);
    }

    char* tok = strtok(p, "/");
    do {
        if (strcmp(tok, ".") != 0) {
            strcat(buf, "/");
            strcat(buf, tok);
        }
        tok = strtok(NULL, "/");
    } while (tok != NULL);

    if (strlen(buf) == 0)
        strcpy(buf, "/");
}

void Process::setAuxVector(int idx, uint64_t type, uint64_t val) {
    auxv[idx].a_type = type;
    auxv[idx].a_un.a_val = val;
}

void Process::notifyChildDied(Process* p) {
    deadChildPID = p->pid;
    for (Thread* t : threads)
        if (t->activeWait && t->activeWait->type == WAIT_FOR_CHILD) {
            klog('d', "Notifying thread %i of dead child %i", t->id, p->pid);
            t->stopWaiting();
        }
}

Process::~Process() {
}
