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
//DEBUG(to_dec(r->eax));
    handlers[r->eax](r);
}


void handlePrint(isrq_registers_t* r) {
    klog((char*)r->ebx);
}

void handleFork(isrq_registers_t* r) {
    int pid = TaskManager::get()->fork();
    r->eax = pid;
}

void handleThread(isrq_registers_t* r) {
    thread_entry_point m = (thread_entry_point)r->ebx;
    void* a = (void*)r->ecx;
    int id = TaskManager::get()->newThread(m, a);
    r->eax = id;
}

void handleDie(isrq_registers_t* r) {
    TaskManager::get()->requestKillProcess(TaskManager::get()->getCurrentThread()->process->pid);
}

void handleWrite(isrq_registers_t* r) {
    FileObject* fo = TaskManager::get()->getCurrentThread()->process->files[(u32int)r->ebx];
    fo->write((char*)r->ecx, 0, r->edx);
    r->eax = r->edx;
}

void handleMemReq(isrq_registers_t* r) {
    u32int t= (u32int)TaskManager::get()->getCurrentThread()->process->requestMemory(r->ebx);
r->eax=t;
}



   typedef short gid_t;
   typedef short uid_t;
   typedef short dev_t;
   typedef short ino_t;
   typedef int mode_t;
   typedef int caddr_t;
struct	stat
{
  dev_t		st_dev;
  ino_t		st_ino;
  mode_t	st_mode;
//  nlink_t	st_nlink;
  uid_t		st_uid;
  gid_t		st_gid;
  dev_t		st_rdev;
//  off_t		st_size;
//  struct timespec st_atim;
//  struct timespec st_mtim;
//  struct timespec st_ctim;
//  blksize_t     st_blksize;
//  blkcnt_t	st_blocks;
};

void handleStat(isrq_registers_t* r) {
*((u32int*)r->ecx +2)= 0020000;
}

void handleOpen(isrq_registers_t* r) {
r->eax=TaskManager::get()->getCurrentThread()->process->openFile(VFS::get()->open((char*)r->ebx, MODE_R));
}

void handleIsTTY(isrq_registers_t* r) {
r->eax=1;
}

void handleClose(isrq_registers_t* r) {
r->eax=1;
}

void SyscallManager::registerDefaults() {
    registerSyscall(0, handlePrint);
    registerSyscall(1, handleFork);
    registerSyscall(2, handleThread);
    registerSyscall(3, handleDie);
    registerSyscall(4, handleWrite);
    registerSyscall(6, handleMemReq);
    registerSyscall(7, handleMemReq);
    registerSyscall(8, handleStat);
    registerSyscall(9, handleIsTTY);
    registerSyscall(11, handleOpen);
}
