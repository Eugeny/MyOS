#include <syscall/SyscallManager.h>
#include <core/TaskManager.h>
#include <core/Thread.h>
#include <kutils.h>
#include <vfs/VFS.h>


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
    TaskManager::get()->pause();
    int scid = r->eax;
    if (STRACE) {
        klogn(">> Syscall #");
        klogn(to_dec(scid));
        klogn(" from PID ");
        klog(to_dec(TaskManager::get()->getCurrentThread()->process->pid));
        klog_flush();
    }
    handlers[scid](r);
    if (STRACE) {
        klogn("<< Syscall #");
        klog(to_dec(scid));
    }
    TaskManager::get()->resume();
}


void handlePrint(isrq_registers_t* r) {
    klog((char*)r->ebx);
}


void sys_newthread(isrq_registers_t* r) {
    thread_entry_point m = (thread_entry_point)r->ebx;
    void* a = (void*)r->ecx;
    int id = TaskManager::get()->newThread(m, a);
    r->eax = id;
}


void sys_sbrk(isrq_registers_t* r) {
    u32int t= (u32int)TaskManager::get()->getCurrentThread()->process->requestMemory(r->ebx);
r->eax=t;
}



   typedef short gid_t;
   typedef short uid_t;
   typedef short dev_t;
   typedef short ino_t;
   typedef int mode_t;
   typedef int caddr_t;

struct  stat 
{
  dev_t         st_dev;
  ino_t         st_ino;
  mode_t        st_mode;
  uid_t         st_uid;
  gid_t         st_gid;
  dev_t         st_rdev;
};




void sys_stat(isrq_registers_t* r) {
  ((stat*)r->ecx)->st_mode= 0x0020000;
}

void sys_open(isrq_registers_t* r) {
r->eax=TaskManager::get()->getCurrentThread()->process->openFile(VFS::get()->open((char*)r->ebx, MODE_R));
}

void sys_isatty(isrq_registers_t* r) {
r->eax=1;
}

void sys_close(isrq_registers_t* r) {
r->eax=1;
}

void sys_fcntl(isrq_registers_t* r) {
    r->eax = 0;
}

void sys_ioctl(isrq_registers_t* r) {
    r->eax = 0;
}

void sys_exit(isrq_registers_t* r) {
    TaskManager::get()->requestKillProcess(TaskManager::get()->getCurrentThread()->process->pid);
}

void sys_fork(isrq_registers_t* r) {
    r->eax = TaskManager::get()->fork();
}

void sys_read(isrq_registers_t* r) {
    FileObject* fo = TaskManager::get()->getCurrentThread()->process->files[(u32int)r->ebx];
    r->eax = fo->read((char*)r->ecx, 0, r->edx);
}

void sys_write(isrq_registers_t* r) {
    FileObject* fo = TaskManager::get()->getCurrentThread()->process->files[(u32int)r->ebx];
//    DEBUG(to_hex(r->ebx));
//    DEBUG(to_hex(r->ecx));
//    DEBUG(to_hex(r->edx));
    fo->write((char*)r->ecx, 0, r->edx);
    r->eax = r->edx;
}

void sys_exec(isrq_registers_t* r) {
    DEBUG((char*)r->ebx);
    DEBUG((char*)r->ecx);
    FileObject* tty = VFS::get()->open((char*)r->ecx, MODE_R|MODE_W);
    Process::create((char*)r->ebx,0,0,tty,tty,tty);
}

void SyscallManager::registerDefaults() {
    registerSyscall(0, handlePrint);
    registerSyscall(1, sys_exit);
    registerSyscall(2, sys_fork);
    registerSyscall(3, sys_read);
    registerSyscall(4, sys_write);
    registerSyscall(5, sys_open);
    registerSyscall(6, sys_close);
    registerSyscall(7, sys_stat);
    registerSyscall(8, sys_isatty);
//    registerSyscall(9, sys_link);
//    registerSyscall(10, sys_unlink);
    registerSyscall(45, sys_sbrk);
    registerSyscall(54, sys_ioctl);
    registerSyscall(55, sys_fcntl);

    registerSyscall(99, sys_newthread);
    registerSyscall(100, sys_exec);
}
