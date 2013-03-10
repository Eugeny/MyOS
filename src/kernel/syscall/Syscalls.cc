#include <syscall/Syscalls.h>
#include <kutil.h>

extern "C" void _syscall_init();

typedef uint64_t (*syscall) (syscall_regs_t*);

static syscall syscalls[1024];

#define SYSCALL(name) uint64_t sys_ ## name (syscall_regs_t* regs) 
#define PROCESS auto process = Scheduler::get()->getActiveThread()->process;
#define THREAD auto thread = Scheduler::get()->getActiveThread();
#define RESOLVE_PATH(var, val) char var[1024]; process->realpath((char*)val, var);
#define WAIT \
    Scheduler::get()->resume();  \
    CPU::STI();                  \
    while (thread->activeWait) { \
        KTRACE                   \
        CPU::halt();             \
    }  

#ifdef KCFG_STRACE
    #define STRACE(format, args...) { \
        __strace_in_progress = true; \
        klog('d', format " from pid %i @ 0x%lx", ## args, \
            Scheduler::get()->getActiveThread()->process->pid, \
            regs->urip); \
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
#include <hardware/vga/VGA.h>
#include <sys/utsname.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/file.h>



SYSCALL(read) {
    PROCESS

    auto fd = regs->rdi;    
    auto buffer = (void*)regs->rsi;
    auto count = (int)regs->rdx;

    STRACE2("read(%i, 0x%x, %i)", fd, buffer, count);

    if (count == -1)
        return 0;

    File* f = process->files[fd];

    if (f->type == FILE_STREAM) {
        int c;
        while (!(c = ((StreamFile*)process->files[fd])->read(buffer, count))) {
            CPU::STI();
            Scheduler::get()->resume();
            CPU::halt();
            Scheduler::get()->pause();
            CPU::CLI();
        }
        return c;
    } else {
        klog('w', "Bad fd type");
        return 0;
    }
}


SYSCALL(write) {
    PROCESS

    auto fd = regs->rdi;    
    auto buffer = (void*)regs->rsi;
    auto count = (int)regs->rdx;

    STRACE2("write(%i, 0x%x, %i)", fd, buffer, count);

    if (count == -1)
        return 0;

    File* f = process->files[fd];

    if (f->type == FILE_STREAM)
        ((StreamFile*)f)->write(buffer, count);

    return count;
}


SYSCALL(open) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)
  
    auto flags = regs->rsi;
    auto mode = regs->rdx;

    STRACE("open('%s', 0x%x, 0x%x)", path, flags, mode);

    if (flags & O_DIRECTORY) {
        auto dir = VFS::get()->opendir(path);
        if (haserr()) 
            return Syscalls::error();
        return process->attachFile(dir);
    } else {
        auto file = VFS::get()->open(path, flags);
        if (haserr()) 
            return Syscalls::error();
        return process->attachFile(file);
    }
}


SYSCALL(close) {
    PROCESS
  
    auto fd = regs->rdi;    

    STRACE("close(%i)", fd);

    process->closeFile(fd);

    return 0;
}


SYSCALL(stat) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)

    auto stat = (struct stat*)regs->rsi;    

    STRACE("stat(%s, 0x%lx)", path, stat);

    File* f = VFS::get()->open(path, O_RDONLY);
    if (haserr()) {
        geterr();
        // dir?
        Directory* d = VFS::get()->opendir(path);
        if (haserr())
            return Syscalls::error();

        d->stat(stat);
        if (haserr()) {
            d->close();
            delete d;
            return Syscalls::error();
        }

        d->close();
        delete d;
        return 0;    
    }

    f->stat(stat);
    if (haserr()) {
        f->close();
        delete f;
        return Syscalls::error();
    }

    f->close();

    delete f;
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

    STRACE2("mmap(0x%lx, 0x%lx, %i, %i, %i, 0x%lx)", addr, length, prot, flags, fd, offset);

    if (length > 0x100000) { // uClibc, U MAD?
        length = 0x80000;
    }

    if (flags | MAP_ANONYMOUS) {
        if (!addr || (addr < KCFG_PAGE_SIZE)) {
            addr = process->brk;
            addr = (addr + KCFG_PAGE_SIZE - 1) / KCFG_PAGE_SIZE * KCFG_PAGE_SIZE;
            process->brk = addr + length;
        }

        uint64_t pflags = PAGEATTR_USER;
        if (flags | MAP_SHARED)
            pflags |= PAGEATTR_SHARED;

        process->addressSpace->allocateSpace(addr, length, pflags); // TODO: flags
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

    STRACE2("munmap(0x%lx, 0x%lx)", addr, length);

    if (length > 0x100000) { // uClibc, U MAD?
        length = 0x80000;
    }

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

    if (oldact && process->signalHandlers[signum])
        *oldact = *process->signalHandlers[signum];

    if (act) {
        if (act->sa_flags & SA_SIGINFO)
            klog('w', "SA_SIGINFO not supported");
        process->setSignalHandler(signum, act);
    }
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


SYSCALL(ioctl) { 
    PROCESS
 
    auto fd = regs->rdi;    
    auto request = regs->rsi;
    auto arg = regs->rdx;

    STRACE("ioctl(%u, %i, 0x%x)", fd, request, arg);

    File* file = process->files[fd];

    if (request == TIOCGWINSZ) {
        auto wsz = (struct winsize*)arg;
        wsz->ws_row = VGA::width;
        wsz->ws_col = VGA::height;
        wsz->ws_xpixel = VGA::width * 8;
        wsz->ws_ypixel = VGA::height * 8;
        return 0;
    } 

    if (request == TCGETS) {
        auto ios = (struct termios*)arg;
        ios->c_iflag = 0;
        ios->c_oflag = 0;
        ios->c_cflag = 0;
        ios->c_lflag = 0;
        ios->c_cc[VEOF] = 4;
        ios->c_cc[VINTR] = 13;
        ios->c_cc[VSUSP] = 32;
        cfsetospeed(ios, 8000);
        cfsetispeed(ios, 8000);
        return 0;
    }

    if (request == TIOCGPGRP) {
        auto res = (uint64_t*)arg;
        *res = process->pgid;
        return 0;
    }

    if (request == TIOCSPGRP) {
        auto res = (uint64_t*)arg;
        process->pgid = *res;
        return 0;
    }

    klog('w', "Unknown ioctl 0x%lx", request);

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
        ((StreamFile*)file)->write(vector->iov_base, vector->iov_len);
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
    process->files[fd2]->refcount++;

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

    Process* p = Scheduler::get()->fork();
    if (p) {
        klog('d', "Forked parent ok");
        return p->pid;
    } else {
        klog('d', "Forked child ok");
        return 0;
    }
}


SYSCALL(exit) { 
    PROCESS
    THREAD

    STRACE("exit()");
    process->requestKill();
    thread->wait(new WaitForever());

    return 0;
}


SYSCALL(wait4) {
    PROCESS
    THREAD
    auto pid = regs->rdi;    
    auto status = (int*)regs->rsi;    
    auto options = regs->rdx;    
    auto usage = (struct rusage*)regs->r10;    

    STRACE("wait4(%i, %lx, %i, %lx)", pid, status, options, usage);

    if (status)
        *status = 0;

    for (Process* p : Scheduler::get()->processes) {
        if (p->ppid == process->pid) {
            thread->wait(new WaitForChild(-1));
            WAIT 
            if (status)
                *status = 0x007f;
            return process->deadChildPID;
        }
    }

    seterr(ECHILD);
    //return -1;
    return Syscalls::error();
}


SYSCALL(kill) { // STUB
    PROCESS
 
    auto pid = regs->rdi;    
    auto signal = regs->rsi;    

    STRACE("kill(%i, %i)", pid, signal);

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


SYSCALL(fcntl) { 
    PROCESS
 
    auto fd = regs->rdi;    
    auto cmd = regs->rsi;
    auto arg = regs->rdx;

    STRACE("fcntl(%u, %i, 0x%x)", fd, cmd, arg);

    File* file = process->files[fd];

    if (cmd == F_DUPFD) {
        for (int i = arg; i < process->files.capacity; i++)
            if (!process->files[i]) {
                process->files[i] = process->files[fd];
                process->files[i]->refcount++;
                return i;
            }
        seterr(EMFILE);
        return Syscalls::error();
    } 

    if (cmd == F_SETFD) {
        // STUB
        return 0;
    }

    klog('w', "Unknown fcntl %i", cmd);

    return 0;
}


SYSCALL(flock) { // STUB
    PROCESS
 
    auto fd = regs->rdi;    
    auto cmd = regs->rsi;

    STRACE("flock(%u, %i)", fd, cmd);
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


SYSCALL(setpgid) {
    auto pid = regs->rdi;    
    auto pgid = regs->rsi;    

    STRACE("setpgid(%i, %i)", pid, pgid);

    Process* p = Scheduler::get()->getProcess(pid);
    if (haserr())
        return Syscalls::error();
    p->pgid = pgid;
    return 0;
}


SYSCALL(getpgrp) {
    PROCESS

    //auto pid = regs->rdi;    

    STRACE("getpgrp()");

    return process->pgid;

    //Process* p = Scheduler::get()->getProcess(pid);
    //if (haserr())
        //return Syscalls::error();
    //return p->pgid;
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


struct kernel_dirent64
{
    uint64_t        d_ino;
    int64_t         d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

SYSCALL(getdents64) { 
    PROCESS
 
    auto fd = regs->rdi;    
    auto buf = (struct kernel_dirent64*)regs->rsi;
    auto sz = regs->rdx; // TODO take in account

    STRACE("getdents64(%u, 0x%lx, %u)", fd, buf, sz);

    auto dir = (Directory*)process->files[fd];
    uint64_t count = 0;

    dirent* de;
    while (de = dir->read()) {
        buf->d_ino = de->d_ino + 5;
        buf->d_reclen = 32;//strlen(de->d_name) + 32;
        buf->d_off = (count += buf->d_reclen);
        strcpy(buf->d_name, de->d_name);
        ///*(char*)(buf + buf->d_reclen - 1) = de->d_type;
        buf->d_type = de->d_type;
        buf += buf->d_reclen;
        return 1;
        //return count;
    }

    return count;
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
    syscalls[0x06] = sys_stat; // lstat
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
    syscalls[0x3c] = sys_exit;
    syscalls[0x3d] = sys_wait4;
    syscalls[0x3e] = sys_kill;
    syscalls[0x3f] = sys_uname;
    syscalls[0x48] = sys_fcntl;
    syscalls[0x49] = sys_flock;
    syscalls[0x4f] = sys_getcwd;
    syscalls[0x50] = sys_chdir;
    syscalls[0x66] = sys_getuid;
    syscalls[0x68] = sys_getuid; // getgid
    syscalls[0x6b] = sys_getuid; // geteuid
    syscalls[0x6c] = sys_getuid; // getegid
    syscalls[0x6d] = sys_setpgid; 
    syscalls[0x6f] = sys_getpgrp;
    syscalls[0x9e] = sys_arch_prctl;
    syscalls[0xc9] = sys_time;
    syscalls[0xd9] = sys_getdents64;
    syscalls[0x6e] = sys_getppid;
}


extern "C" uint64_t _syscall_handler(syscall_regs_t* regs) {
    uint64_t result = 0;

    Scheduler::get()->pause();
    geterr(); // drop errors

    if (syscalls[regs->id]) {
        __strace_in_progress = false;
        result = syscalls[regs->id](regs);
        #ifdef KCFG_STRACE
            if (__strace_in_progress) {
                klog('d', " = %lx", result);
                klog_flush();
            }
        #endif
    } else {
        __outputhex(regs->id, 70);
        klog('w', "Unknown syscall");
        klog_flush();
        klog('w', "0x%lx", regs->id);
        klog_flush();
        //for(;;);
    }

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
