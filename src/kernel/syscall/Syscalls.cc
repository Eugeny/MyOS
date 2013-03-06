#include <syscall/Syscalls.h>
#include <kutil.h>

extern "C" void _syscall_init();

typedef uint64_t (*syscall) (syscall_regs_t*);

static syscall syscalls[1024];

#define SYSCALL(name) uint64_t sys_ ## name (syscall_regs_t* regs) 
#define PROCESS Process* process = Scheduler::get()->getActiveThread()->process;

#ifdef KCFG_STRACE
    #define STRACE(format, args...) { \
        __strace_in_progress = true; \
        klog('t', format " from 0x%lx", ## args, regs->urip); \
        klog_flush(); \
    }
#else
    #define STRACE(format, args...) {}
#endif

#ifdef KCFG_STRACE2
    #define STRACE2 STRACE
#else
    #define STRACE2(format, args...) {}
#endif

static bool __strace_in_progress = false;

//-----------------------------------
//-----------------------------------

#include <fs/vfs/VFS.h>
#include <core/CPU.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <sys/utsname.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>



SYSCALL(read) {
    PROCESS

    auto fd = regs->rdi;    
    auto buffer = (void*)regs->rsi;
    auto count = (int)regs->rdx;

    STRACE2("read(%i, 0x%x, %i)", fd, buffer, count);

    if (count == -1)
        return 0;

    int c;
    while (!(c = process->files[fd]->read(buffer, count))) {
        CPU::STI();
        Scheduler::get()->resume();
        CPU::halt();
        Scheduler::get()->pause();
        CPU::CLI();
    }
    return c;
}


SYSCALL(write) {
    PROCESS

    auto fd = regs->rdi;    
    auto buffer = (void*)regs->rsi;
    auto count = (int)regs->rdx;

    STRACE2("write(%i, 0x%x, %i)", fd, buffer, count);

    if (count == -1)
        return 0;

    process->files[fd]->write(buffer, count);

    return count;
}


SYSCALL(open) {
    PROCESS
  
    auto path = (char*)regs->rdi;    
    auto flags = regs->rsi;
    auto mode = regs->rdx;

    STRACE("open('%s', 0x%x, 0x%x)", path, flags, mode);

    auto file = VFS::get()->open(path, flags);

    if (haserr()) 
        return Syscalls::error();

    return process->attachFile(file);
}


SYSCALL(close) {
    PROCESS
  
    auto fd = regs->rdi;    

    STRACE("close(%i)", fd);

    process->closeFile(fd);

    return 0;
}


SYSCALL(stat) {
    auto path = (char*)regs->rdi;    
    auto stat = (struct stat*)regs->rsi;    

    STRACE("stat(%s, 0x%lx)", path, stat);

    File* f = VFS::get()->open(path, O_RDONLY);
    if (haserr())
        return Syscalls::error();

    f->stat(stat);
    if (haserr()) {
        f->close();
        return Syscalls::error();
    }

    f->close();

    return 0;
}


SYSCALL(fstat) {
    PROCESS
  
    auto fd = regs->rdi;    
    auto stat = (struct stat*)regs->rsi;    

    STRACE("fstat(%i, 0x%lx)", fd, stat);

    process->files[fd]->stat(stat);

    return 0;
}


SYSCALL(mmap) {
    PROCESS
  
    auto addr = regs->rdi;
    auto length = regs->rsi;
    auto prot = regs->rdx;
    auto flags = regs->r10;
    auto fd = regs->r8;    
    auto offset = regs->r9;    

    STRACE("mmap(0x%lx, 0x%lx, %i, %i, %i, 0x%lx)", addr, length, prot, flags, fd, offset);

    if (flags | MAP_ANONYMOUS) {
        if (!addr || (addr < KCFG_PAGE_SIZE)) {
            addr = process->brk;
            addr = (addr + KCFG_PAGE_SIZE - 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
            process->brk = addr + length;

            //addr &= 0xFFFFFFFFFF000000; // FUCK MALLOC
        }
        //return (uint64_t)MAP_FAILED;
        process->addressSpace->allocateSpace(addr, length, PAGEATTR_USER); // TODO: flags
        process->addressSpace->namePage(process->addressSpace->getPage(addr, false), "mmap()");

        memset((void*)addr, 0, length);
        return addr;
    } else {
        klog('e', "Unsupported mmap(flags=%i)", flags);
        klog_flush();
        for(;;);
    }

    return 0;
}


SYSCALL(munmap) {
    PROCESS
  
    auto addr = regs->rdi;
    auto length = regs->rsi;

    STRACE("munmap(0x%lx, 0x%lx)", addr, length);

    process->addressSpace->releaseSpace(addr, length); // TODO: flags

    return 0;
}


SYSCALL(brk) {
    PROCESS
    
    auto addr = regs->rdi;    

    STRACE("brk(0x%x)", addr);

    if (addr > 0)
        process->sbrk(addr - process->brk);

    return process->brk;
}


SYSCALL(rt_sigaction) { // STUB
    PROCESS

    auto signum = regs->rdi;
    auto act = (struct sigaction *)regs->rsi;
    auto oldact = (struct sigaction *)regs->rdx;

    STRACE("rt_sigaction(%i, 0x%x, 0x%x)", signum, act, oldact); 

    return 0;
}


SYSCALL(sigprocmask) { // STUB
    PROCESS

    auto how = regs->rdi;
    auto set = (sigset_t*)regs->rsi;
    auto oldset = (sigset_t*)regs->rdx;

    STRACE("sigprocmask(%i, 0x%x, 0x%x)", how, set, oldset);

    return 0;
}


SYSCALL(ioctl) { // STUB
    PROCESS
 
    auto fd = regs->rdi;    
    auto request = regs->rsi;
    auto arg = regs->rdx;

    STRACE("ioctl(%u, %i, 0x%x)", fd, request, arg);

    File* file = process->files[fd];

    return 0;
}


SYSCALL(writev) {
    PROCESS
 
    auto fd = regs->rdi;    
    auto iov = (uint64_t)regs->rsi;
    auto iovcnt = regs->rdx;

    STRACE("writev(%i, 0x%x, %i)", fd, iov, iovcnt);

    File* file = process->files[fd];

    uint64_t written = 0;

    for (uint64_t i = 0; i < iovcnt; i++) {
        auto vector = (struct iovec*)(iov + i * sizeof(iovec));
        file->write(vector->iov_base, vector->iov_len);
        written += vector->iov_len;
    }

    return written;
}


SYSCALL(dup2) {
    PROCESS
 
    auto fd = regs->rdi;    
    auto fd2 = regs->rsi;    

    STRACE("dup2(%i, %i)", fd, fd2);

    if (process->files[fd2])
        process->closeFile(fd2);
    process->files[fd2] = process->files[fd];

    return fd2;
}


SYSCALL(getpid) {
    PROCESS
    STRACE("getpid()");
    return process->pid;
}


SYSCALL(fork) { 
    PROCESS
 
    STRACE("fork()");

    if (Scheduler::get()->fork()) {
        klog('w', "FORK OUT 1");
        return Scheduler::get()->getActiveThread()->process->pid;
    } else {
        klog('w', "FORK OUT 2");
        return 0;
    }
}


SYSCALL(wait4) { // STUB
    PROCESS
 
    auto pid = regs->rdi;    
    auto status = (int*)regs->rsi;    
    auto options = regs->rdx;    
    auto usage = (struct rusage*)regs->r10;    

    STRACE("wait4(%i, %lx, %i, %lx)", pid, status, options, usage);

    return 0;
}


SYSCALL(uname) {
    auto buf = (struct utsname*)regs->rdi;
    
    STRACE("uname(%lx)", buf);

    strcpy(buf->sysname, "Linux");
    strcpy(buf->nodename, "Test");
    strcpy(buf->release, "3.2.0-35-generic");
    strcpy(buf->version, "#55-Ubuntu SMP Wed Dec 5 17:42:16 UTC 2012");
    strcpy(buf->machine, "x86_64");

    return 0;
}


SYSCALL(getcwd) {
    PROCESS
    
    auto buf = (char*)regs->rdi;    
    auto size = regs->rsi;   
    
    STRACE("getcwd(0x%lx, 0x%lx)", buf, size);

    strncpy(buf, process->cwd, size);
    return (uint64_t)buf;
}


SYSCALL(chdir) {
    PROCESS
    
    auto path = (char*)regs->rdi;    
    
    STRACE("chdir(%s)", path);

    Directory* dir = VFS::get()->opendir(path);
    if (haserr())
        return Syscalls::error();
    dir->close();

    strcpy(process->cwd, path);
    return 0;
}


SYSCALL(getuid) {
    STRACE("getuid()");
    return 0;
}


SYSCALL(arch_prctl) {
    PROCESS
    
    auto code = regs->rdi;    
    auto addr = regs->rsi;    

    STRACE("arch_prctl(0x%x, 0x%lx)", code, addr);

    if (code == 0x1002) {
        klog('t', "Writing FSBASE=0x%lx", addr);
        CPU::WRMSR(MSR_FSBASE, addr);
    } else {
        klog('e', "Unsupported arch_prctl(code=%i)", code);
        klog_flush();
        for(;;);
    }

    return 0;
}


SYSCALL(time) { // STUB
    PROCESS
    
    auto timeptr = (time_t*)regs->rdi;    
    time_t timev = 0;
    
    STRACE("time(0x%lx)", timeptr);

    if (timeptr)
        *timeptr = timev;

    return timev;
}


SYSCALL(getppid) {
    PROCESS
    STRACE("getppid()");
    return process->ppid;
}


//-----------------------------------
//-----------------------------------




void Syscalls::init() {
    _syscall_init();
    for (uint64_t i = 0; i < 1024; i++)
        syscalls[i] = NULL;

    syscalls[0x00] = sys_read;
    syscalls[0x01] = sys_write;
    syscalls[0x02] = sys_open;
    syscalls[0x03] = sys_close;
    syscalls[0x04] = sys_stat;
    syscalls[0x05] = sys_fstat;
    syscalls[0x09] = sys_mmap;
    syscalls[0x0b] = sys_munmap;
    syscalls[0x0c] = sys_brk;
    syscalls[0x0d] = sys_rt_sigaction;
    syscalls[0x0e] = sys_sigprocmask;
    syscalls[0x10] = sys_ioctl;
    syscalls[0x14] = sys_writev;
    syscalls[0x21] = sys_dup2;
    syscalls[0x27] = sys_getpid;
    syscalls[0x39] = sys_fork;
    syscalls[0x3d] = sys_wait4;
    syscalls[0x3f] = sys_uname;
    syscalls[0x4f] = sys_getcwd;
    syscalls[0x50] = sys_chdir;
    syscalls[0x66] = sys_getuid;
    syscalls[0x68] = sys_getuid; // getgid
    syscalls[0x6b] = sys_getuid; // geteuid
    syscalls[0x6c] = sys_getuid; // getegid
    syscalls[0x9e] = sys_arch_prctl;
    syscalls[0xc9] = sys_time;
    syscalls[0x6e] = sys_getppid;
}


extern "C" uint64_t _syscall_handler(syscall_regs_t* regs) {
    uint64_t result = 0;

    Scheduler::get()->pause();

    if (syscalls[regs->id]) {
        __strace_in_progress = false;
        result = syscalls[regs->id](regs);
        #ifdef KCFG_STRACE
            if (__strace_in_progress) {
                klog('t', " = %lx", result);
                klog_flush();
            }
        #endif
    } else {
        __outputhex(regs->id, 70);
        klog('w', "Unknown syscall");
        klog_flush();
        klog('w', "0x%lx", regs->id);
        klog_flush();
        for(;;);
    }

    //dump_stack(regs->ursp, regs->rbp);

    Scheduler::get()->resume();

    //for(int i =0;i<500000;i++);
    return result;
}

uint64_t Syscalls::error() {
    int r = -geterr();
    #ifdef KCFG_STRACE
        //klog('t', " = (err) %i", r);
        //klog_flush();
    #endif   
    return (uint64_t)r;
}
