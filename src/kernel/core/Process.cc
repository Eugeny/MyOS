#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>
#include <string.h>
#include <signal.h>
#include <lang/libc/libc-ext.h>


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
    stackbrk = 0x800000000000 - 0x2000;
    isKernel = false;
    isPaused = false;
    pty = NULL;

    strcpy(exeName, "");
    strcpy(this->name, name);
    strcpy(cwd, "/");
}

Process* Process::clone() {
    Process* p = new Process(*this);
    for (auto f : files)
        f->refcount++;
    p->threads.clear();
    p->pid = makepid();
    p->signalHandlers.clear();
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
    addressSpace->allocateSpace(base, size, isKernel ? 0 : (PAGEATTR_USER|PAGEATTR_COPY));
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
    klog('t', "Queueing signal %i on pid %i", signal, pid);
    pendingSignals |= (1 << signal);
}

typedef void (*sa_handler_t)(int);

void Process::runPendingSignals() {
    for (int i = 0; i < 31; i++) {
        if (pendingSignals & (1 << i)) {
            klog('t', "Executing signal %i on pid %i", i, pid);
            if (signalHandlers[i]) {
                sa_handler_t handler = signalHandlers[i]->sa_handler;
                if (handler == SIG_DFL) {
                    executeDefaultSignal(i);
                } else if (handler == SIG_IGN) ;
                else 
                    handler(i);
            } else {
                executeDefaultSignal(i);
            }
        }
    }
    pendingSignals = 0;
}

void Process::executeDefaultSignal(int signal) {
    klog('t', "Executing default signal handler %i on pid %i", signal, pid);
    if (signal == SIGSTOP) {
        isPaused = true;
        if (parent) {
            parent->queueSignal(SIGCHLD);
            parent->notifyChildDied(this, 0x007f);
        }
    }
    if (signal == SIGKILL) {
        isPaused = true;
        requestKill();
    }   
    if (signal == SIGCONT)
        isPaused = false;
    if (signal == SIGTERM || signal == SIGINT) {
        requestKill();
    }
}

Thread* Process::spawnThread(threadEntryPoint entry, const char* name) {
    Thread* t = new Thread(this, name);
    t->state = Scheduler::get()->kernelThread->state;
    for (Thread* thread : threads) {
        t->state = thread->state;
        break;
    }
    t->state.addressSpace = addressSpace;
    t->createStack(0x400000);
    t->pushOnStack(0);
    t->pushOnStack(0);
    t->state.regs.rip = (uint64_t)entry;
    threads.add(t);
    Scheduler::get()->registerThread(t);
    return t;
}

int Process::attachFile(File* f) {
    f->refcount++;
    auto fd = files.add(f);
    klog('t', "Attaching FD %i of type %i to process %i", fd, f->type, pid);
    return fd;
}

void Process::closeFile(int fd) {
    File* f = files[fd];
    klog('t', "Detaching FD %i of type %i from process %i", fd, f->type, pid);
    f->refcount--;
    f->fdClosed();
    files[fd] = NULL;
    if (f->refcount == 0) {
        klog('t', "Reaping FD %i", fd);
        f->close();
        delete f;
    }
}

void Process::realpath(char* p, char* buf) {
    normalize_path(cwd, p, buf);
    return;

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

void Process::notifyChildDied(Process* p, uint64_t status) {
    deadChildPID = p->pid;
    deadChildStatus = status;
    for (Thread* t : threads)
        if (t->activeWait) {
            if (t->activeWait->type == WAIT_FOR_CHILD) {
                klog('d', "Notifying thread %i of dead child %i", t->id, p->pid);
                t->stopWaiting();
            }
        }
}

Process::~Process() {
    for (int i = 0; i < files.capacity; i++)
        if (files[i])
            closeFile(i);
    for (auto h : signalHandlers)
        delete h;
    delete addressSpace;
}
