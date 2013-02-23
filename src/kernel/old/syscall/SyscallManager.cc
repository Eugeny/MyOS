#include <syscall/SyscallManager.h>
#include <core/TaskManager.h>
#include <core/Thread.h>
#include <kutils.h>
#include <vfs/VFS.h>
#include <core/Wait.h>
#include <stat.h>


void handler(isrq_registers_t* r) {
    SyscallManager::get()->handleSyscall(r);
}

void SyscallManager::init() {
    memset(handlers, 0, sizeof(interrupt_handler) * 256);
    Interrupts::get()->setHandler(128, handler);
}

void SyscallManager::registerSyscall(int idx, interrupt_handler h) {
    handlers[idx] = h;
}

void SyscallManager::handleSyscall(isrq_registers_t* r) {
//    TaskManager::get()->pause();
    int scid = r->eax;

    #ifdef STRACE
    klogn("** ");
    klogn(to_dec(TaskManager::get()->getCurrentThread()->process->pid));
    klogn(" ");
    klog_flush();
    #endif

    if (handlers[scid] != 0)
        handlers[scid](r);
    else {
        #ifdef STRACE
        klogn("#");
        klogn(to_dec(scid));
        #endif
    }

    #ifdef STRACE
    klog("<<");
    klog_flush();
    #endif

    TaskManager::get()->resume();
}


void handlePrint(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("kprint(");
    klogn("str=");
    klogn((char*)r->ebx);
    klogn(")");
    #endif

    klog((char*)r->ebx);
}


void sys_newthread(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("newthread(");
    klogn("entry=");
    klogn(to_hex((u32int)r->ebx));
    klogn(", arg=");
    klogn(to_hex((u32int)r->ecx));
    klogn(")");
    #endif

    thread_entry_point m = (thread_entry_point)r->ebx;
    void* a = (void*)r->ecx;
    int id = TaskManager::get()->newThread(m, a);
    r->eax = id;
}


void sys_sbrk(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("sbrk(");
    klogn("size=");
    klogn(to_hex((u32int)r->ebx));
    klogn(")");
    #endif

    u32int t= (u32int)TaskManager::get()->getCurrentThread()->process->requestMemory(r->ebx);

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)t));
    #endif

    r->eax=t;
}


void sys_waitpid(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("waitpid(");
    klogn("pid=");
    klogn(to_dec((u32int)r->ebx));
    klogn(")");
    #endif

    TaskManager::get()->getCurrentThread()->wait = new WaitPID(r->ebx);
    TaskManager::get()->nextTask();
}


void sys_stat(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("stat(");
    klogn("fd=");
    klogn(to_dec((u32int)r->ebx));
    klogn(", stat_t=");
    klogn(to_hex((u32int)r->ecx));
    klogn(")");
    #endif
    ((stat*)r->ecx)->st_mode= 0x0020000;
}

void sys_fstat(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("stat(");
    klogn("fd=");
    klogn(to_dec((u32int)r->ebx));
    klogn(", stat_t=");
    klogn(to_hex((u32int)r->ecx));
    klogn(")");
    #endif
    ((stat*)r->ecx)->st_mode= 0x0020000;
}

void sys_open(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("open(");
    klogn("path=");
    klogn((char*)r->ebx);
    klogn(", flags=");
    klogn(to_hex((u32int)r->ecx));
    klogn(", mode=");
    klogn(to_hex((u32int)r->edx));
    klogn(")");
    #endif

    r->eax=TaskManager::get()->getCurrentThread()->process->openFile(VFS::get()->open((char*)r->ebx, r->edx));

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_isatty(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("isatty(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(")");
    #endif

    r->eax=1; //TODO
}

void sys_close(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("close(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(")");
    #endif

    r->eax=1; //TODO
}

void sys_fcntl(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("fcntl(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(", cmd=");
    klogn(to_hex((u32int)r->ecx));
    klogn(", arg=");
    klogn(to_hex((u32int)r->edx));
    klogn(")");
    #endif

    r->eax = 0;

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_ioctl(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("fcntl(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(", cmd=");
    klogn(to_hex((u32int)r->ecx));
    klogn(", arg=");
    klogn(to_hex((u32int)r->edx));
    klogn(")");
    #endif

    r->eax = 0;

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_exit(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("exit(");
    klogn("code=");
    klogn(to_dec(r->ebx));
    klogn(")");
    #endif
    TaskManager::get()->requestKillProcess(TaskManager::get()->getCurrentThread()->process->pid);
}

void sys_fork(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("fork()");
    klogn(")");
    #endif

    r->eax = TaskManager::get()->fork();

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_read(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("read(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(", buf=");
    klogn(to_hex((u32int)r->ecx));
    klogn(", count=");
    klogn(to_hex((u32int)r->edx));
    klogn(")");
    #endif

    FileObject* fo = TaskManager::get()->getCurrentThread()->process->files[(u32int)r->ebx];

    __asm__("sti");
    r->eax = fo->read((char*)r->ecx, 0, r->edx);
    __asm__("cli");

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_write(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("write(");
    klogn("fd=");
    klogn(to_dec(r->ebx));
    klogn(", buf=");
    klogn(to_hex((u32int)r->ecx));
    klogn(", count=");
    klogn(to_hex((u32int)r->edx));
    klogn(")");
    #endif

    FileObject* fo = TaskManager::get()->getCurrentThread()->process->files[(u32int)r->ebx];
    fo->write((char*)r->ecx, 0, r->edx);
    r->eax = r->edx;

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_exec(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("exec(");
    klogn("path=");
    klogn((char*)(r->ebx));
    klogn(", stdout=");
    klogn((char*)r->ecx);
    klogn(")");
    #endif

    FileObject *stdin, *stdout, *stderr;

    if (r->ecx == 0) {
        stdin  = TaskManager::get()->getCurrentThread()->process->files[0];
        stdout = TaskManager::get()->getCurrentThread()->process->files[1];
        stderr = TaskManager::get()->getCurrentThread()->process->files[2];
    } else {
        stdin = stdout = stderr = VFS::get()->open((char*)r->ecx, MODE_R|MODE_W);
    }

    r->eax = Process::create((char*)r->ebx, r->edx, (char**)(r->esi), stdin, stdout, stderr);

    #ifdef STRACE
    klogn(" = ");
    klogn(to_hex((u32int)r->eax));
    #endif
}

void sys_opendir(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("opendir(");
    klogn("path=");
    klogn((char*)(r->ebx));
    klogn(")");
    #endif

    r->eax=TaskManager::get()->getCurrentThread()->process->openDir((char*)r->ebx);
}

void sys_readdir(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("readdir(");
    klogn("dfd=");
    klogn(to_dec(r->ebx));
    klogn(")");
    #endif

    r->eax = (u32int)TaskManager::get()->getCurrentThread()->process->dirs[r->ebx]->read();
}

void sys_closedir(isrq_registers_t* r) {
    #ifdef STRACE
    klogn("closedir(");
    klogn("path=");
    klogn((char*)(r->ebx));
    klogn(")");
    #endif

    TaskManager::get()->getCurrentThread()->process->closeDir(r->ebx);
}


void SyscallManager::registerDefaults() {
    registerSyscall(0, handlePrint);
    registerSyscall(1, sys_exit);
    registerSyscall(2, sys_fork);
    registerSyscall(3, sys_read);
    registerSyscall(4, sys_write);
    registerSyscall(5, sys_open);
    registerSyscall(6, sys_close);
    registerSyscall(7, sys_waitpid);
    registerSyscall(8, sys_isatty);
//    registerSyscall(9, sys_link);
//    registerSyscall(10, sys_unlink);
    registerSyscall(18, sys_stat);
    registerSyscall(28, sys_fstat);
    registerSyscall(45, sys_sbrk);
    registerSyscall(54, sys_ioctl);
    registerSyscall(55, sys_fcntl);

    registerSyscall(99, sys_newthread);
    registerSyscall(100, sys_exec);
    registerSyscall(101, sys_opendir);
    registerSyscall(102, sys_readdir);
    registerSyscall(103, sys_closedir);
}
