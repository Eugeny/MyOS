#include <core/CPU.h>
#include <core/Process.h>
#include <core/Scheduler.h>
#include <elf/ELF.h>
#include <fs/vfs/VFS.h>
#include <hardware/cmos/CMOS.h>
#include <hardware/pm.h>
#include <hardware/vga/VGA.h>
#include <kutil.h>
#include <memory/FrameAlloc.h>
#include <syscall/Syscalls.h>


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
#define WAITONE \
    CPU::STI();                     \
    Scheduler::get()->resume();     \
    CPU::halt();                    \
    Scheduler::get()->pause();      \
    CPU::CLI();

#ifdef KCFG_STRACE
    #define STRACE(format, args...) { \
        __strace_in_progress = true; \
        klog('t', "--sys-- " format " from:   process %i   thread %i   pgid %i @ 0x%lx", ## args, \
            Scheduler::get()->getActiveThread()->process->pid, \
            Scheduler::get()->getActiveThread()->id, \
            Scheduler::get()->getActiveThread()->process->pgid, \
            regs->urip); \
    }
#else
    #define STRACE(format, args...) {}
#endif

#ifdef KCFG_STRACE2
    #define STRACE2 STRACE
#else
    #define STRACE2(format, args...) {}
#endif

#ifdef KCFG_WARN_SYSCALL_STUBS
    #define WARN_STUB(msg) klog('w', "Syscall stub: " #msg);
#else
    #define WARN_STUB(msg) klog('t', "Syscall stub: " #msg);
#endif


static bool __strace_in_progress = false;

//-----------------------------------
//-----------------------------------

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <termios.h>
#include <time.h>



SYSCALL(read) {
    PROCESS

    auto fd = regs->rdi;    
    auto buffer = (void*)regs->rsi;
    auto count = (int)regs->rdx;

    STRACE2("read(%i, 0x%x, %i)", fd, buffer, count);

    if (count == -1)
        return 0;

    auto f = (StreamFile*)process->files[fd];

    if (f->type == FILE_STREAM) {
        int c;
        while (!f->isEOF() && !(c = f->read(buffer, count))) {
            CPU::STI();
            Scheduler::get()->resume();
            CPU::halt();
            Scheduler::get()->pause();
            CPU::CLI();
        }
        return c;
    } else {
        klog('w', "Bad fd type %i", f->type);
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
        count = ((StreamFile*)f)->write(buffer, count);
    else {
        klog('w', "Bad fd type %i", f->type);
        seterr(EBADF);
        return Syscalls::error();
    }

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


SYSCALL(poll) {
    PROCESS
  
    auto fds = (struct pollfd*)regs->rdi;    
    auto nfds = regs->rsi;
    auto timeout = regs->rdx;    

    STRACE("poll(0x%lx, %i, %i)", fds, nfds, timeout);

    while (true) {
        for (uint i = 0; i < nfds; i++) {
            auto f = (StreamFile*)process->files[fds[i].fd];

            if (f->type == FILE_STREAM) {
                if (f->canRead()) {
                    fds[i].revents = POLLIN;
                    return 1;
                }
            }
        }

        WAITONE
    }

    return 0;
}


SYSCALL(lseek) {
    PROCESS
  
    auto fd = regs->rdi;    
    auto offset = regs->rsi;    
    auto whence = regs->rdx;    

    STRACE("lseek(%i, %i, %i)", fd, offset, whence);

    File* f = process->files[fd];

    if (f->type == FILE_STREAM)
        return ((StreamFile*)f)->seek(offset, whence);
    else {
        klog('w', "Bad fd type %i", f->type);
        seterr(EBADF);
        return Syscalls::error();
    }


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


SYSCALL(rt_sigaction) {
    PROCESS

    auto signum = regs->rdi;
    auto act = (struct sigaction *)regs->rsi;
    auto oldact = (struct sigaction *)regs->rdx;

    STRACE("rt_sigaction(%i, 0x%x, 0x%x)", signum, act, oldact); 

    // bad oldact causes stack corruption
    //if (oldact && process->signalHandlers[signum])
    //    *oldact = *process->signalHandlers[signum];

    if (act) {
        if (act->sa_flags & SA_SIGINFO)
            WARN_STUB("SA_SIGINFO not supported");
        process->setSignalHandler(signum, act);
    }
    return 0;
}


SYSCALL(sigprocmask) { 
//    PROCESS

    auto how = regs->rdi;
    auto set = (sigset_t*)regs->rsi;
    auto oldset = (sigset_t*)regs->rdx;

    STRACE("sigprocmask(%i, 0x%x, 0x%x)", how, set, oldset);
    WARN_STUB("sigprocmask");

    return 0;
}


SYSCALL(ioctl) { 
    PROCESS
 
    auto fd = regs->rdi;    
    auto request = regs->rsi;
    auto arg = regs->rdx;

    STRACE("ioctl(%u, %x, 0x%x)", fd, request, arg);

    //File* file = process->files[fd];

    if (request == TIOCGWINSZ) {
        STRACE("TIOCGWINSZ");
        auto wsz = (struct winsize*)arg;
        wsz->ws_row = VGA::width;
        wsz->ws_col = VGA::height;
        wsz->ws_xpixel = VGA::width * 8;
        wsz->ws_ypixel = VGA::height * 8;
        return 0;
    } 

    if (request == TCGETS) {
        STRACE("TCGETS");
        auto ios = (struct termios*)arg;
        ios->c_iflag = 0;
        ios->c_oflag = 0;
        ios->c_cflag = 0;
        ios->c_lflag = ECHO | ISIG;
        ios->c_cc[VMIN] = 0;
        ios->c_cc[VTIME] = 0;
        ios->c_cc[VEOF] =  'd' - 64; // Ctrl-D
        ios->c_cc[VINTR] = 'c' - 64; // Ctrl-C
        ios->c_cc[VSUSP] = 'z' - 64; // Ctrl-Z
        cfsetospeed(ios, 8000);
        cfsetispeed(ios, 8000);
        return 0;
    }

    if (request == TIOCGPGRP) {
        STRACE("TIOCGPGRP");
        auto res = (uint64_t*)arg;
        *res = process->pgid;
        return 0;
    }

    if (request == TIOCSPGRP) {
        STRACE("TIOCSPGRP");
        auto res = (uint64_t*)arg;
        process->pgid = *res;
        return 0;
    }

    if (request == TCFLSH) {
        STRACE("TCFLSH");
        return 0;
    }

    if (request == TCSETS || request == TCSETSW) {
        WARN_STUB("TCSETSx");
        //auto ios = (struct termios*)arg;
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


SYSCALL(access) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)
    auto mode = regs->rsi;

    STRACE("access(%s, %i)", path, mode);
    WARN_STUB("access");

    int access = S_IRWXU | S_IRWXG | S_IRWXO;
    return access;
}

SYSCALL(pipe) { // FIXME pipe needs separate end fds?
    PROCESS
 
    auto fds = (int*)regs->rdi;    

    STRACE("pipe(0x%lx)", fds);

    auto pipe = new Pipe();

    fds[0] = process->attachFile(pipe);
    fds[1] = process->attachFile(pipe);

    STRACE("pipe FDs = %i, %i", fds[0], fds[1]);
    STRACE("FD type %i", process->files[fds[0]]->type);

    return 0;
}


SYSCALL(dup) {
    PROCESS
 
    auto fd = regs->rdi;    

    STRACE("dup(%i)", fd);

    for (int i = 0; i < process->files.capacity; i++)
        if (!process->files[i]) {
            process->files[i] = process->files[fd];
            STRACE("FD type %i", process->files[i]->type);
            process->files[i]->refcount++;
            return i;
        }

    seterr(EMFILE);
    return Syscalls::error();
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


SYSCALL(nanosleep) {
    PROCESS
    THREAD

    auto req = (struct timespec*)regs->rdi;
    auto rem = (struct timespec*)regs->rdi;

    STRACE("nanosleep(0x%lx, 0x%lx)", req, rem);

    uint64_t msdelay = req->tv_sec * 1000 + req->tv_nsec / 1000;
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

    thread->wait(new WaitForDelay(msdelay));
    WAIT;

    return process->pid;
}


SYSCALL(getpid) {
    PROCESS
    STRACE("getpid()");
    return process->pid;
}


SYSCALL(fork) { 
    STRACE("fork()");

    Process* p = Scheduler::get()->fork();
    if (p) {
        klog('t', "Forked parent ok");
        return p->pid;
    } else {
        klog('t', "Forked child ok");
        return 0;
    }
}


SYSCALL(vfork) { 
    THREAD
    STRACE("vfork()");

    Process* p = Scheduler::get()->fork();
    if (p) {
        klog('t', "VForked parent ok");
        thread->wait(new WaitForChild(-1));
        WAIT 
        return p->pid;
    } else {
        klog('t', "VForked child ok");
        return 0;
    }
}


SYSCALL(execve) { 
    PROCESS
    THREAD
    RESOLVE_PATH(path, regs->rdi)
    
    auto argv = (char**)regs->rsi;
    auto envp = (char**)regs->rdx;

    STRACE("execve(%s, 0x%lx, 0x%lx)", path, argv, envp);

    // Stack will be destroyed; backup argv & envp
    #define BUFSIZE 256
    #define STRSIZE 256
    auto n_argv = new char*[BUFSIZE];
    auto n_envp = new char*[BUFSIZE];

    for (int i = 0; i < BUFSIZE; i++) {
        if (argv[i] != NULL) {
            n_argv[i] = new char[STRSIZE];
            strcpy(n_argv[i], argv[i]);
        } else {
            n_argv[i] = NULL;
            break;
        }
    }

    if (envp) {
        for (int i = 0; i < BUFSIZE; i++) {
            if (envp[i] != NULL) {
                klog('t', "Setting nenvp %i", i);
                n_envp[i] = new char[STRSIZE];
                strcpy(n_envp[i], envp[i]);
            } else {
                n_envp[i] = NULL;
                break;
            }
        }
    }

    auto elf = new ELF();
    elf->loadFromFile(path);

    if (!haserr()) {
        strcpy(process->name, (char*)regs->rdi);
        elf->loadIntoProcess(process);
        elf->startMainThread(process, n_argv, n_envp);  
    }    
    
    delete elf;
    
    for (int i = 0; i < BUFSIZE; i++)
        if (n_argv[i] != NULL)
            delete n_argv[i];
        else
            break;
    
    if (envp)
        for (int i = 0; i < BUFSIZE; i++)
            if (n_envp[i] != NULL) {
                klog('t', "unSetting nenvp %i", i);

                delete n_envp[i];
            } else
                break;
     
    delete n_argv;
    delete n_envp;

    if (haserr()) {
        return Syscalls::error();
    }

    Scheduler::get()->requestKill(thread);
    Scheduler::get()->waitForNextTask();

    return 0;
}


SYSCALL(exit) { 
    PROCESS

    STRACE("exit()");
    Scheduler::get()->requestKill(process);
    Scheduler::get()->waitForNextTask();

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
                //*status = process->deadChildStatus;
                *status = 0x007f;
            return process->deadChildPID;
        }
    }

    seterr(ECHILD);
    //return -1;
    return Syscalls::error();
}


SYSCALL(kill) {
    PROCESS

    auto pid = (int)regs->rdi;    
    auto signal = regs->rsi;    

    STRACE("kill(%i, %i)", pid, signal);

    if (pid == 0) {
        STRACE("killing pgid %i", process->pgid);
        for (auto p : Scheduler::get()->processes)
            if (p->pgid == process->pgid) {
                p->queueSignal(signal);
                WAITONE
            }
        return 0;
    }

    if (pid == -1) {
        for (auto p : Scheduler::get()->processes)
            if (p->pid > 1) {
                p->queueSignal(signal);
                WAITONE
            }
        return 0;
    }

    if (pid < -1) {
        WARN_STUB("kill(<-1, ...)");
        return 0;
    }

    Scheduler::get()->getProcess(pid)->queueSignal(signal);
    WAITONE 

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
        STRACE("F_DUPFD");
        for (int i = arg; i < process->files.capacity; i++)
            if (!process->files[i]) {
                process->files[i] = file;
                process->files[i]->refcount++;
                return i;
            }
        seterr(EMFILE);
        return Syscalls::error();
    } 

    if (cmd == F_SETFD) {
        WARN_STUB("F_SETFD");
        return 0;
    }

    klog('w', "Unknown fcntl %i", cmd);

    return 0;
}


SYSCALL(flock) {
    auto fd = regs->rdi;    
    auto cmd = regs->rsi;

    STRACE("flock(%u, %i)", fd, cmd);
    WARN_STUB("flock");
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
    RESOLVE_PATH(path, regs->rdi)
    
    STRACE("chdir(%s)", path);

    Directory* dir = VFS::get()->opendir(path);
    if (haserr())
        return Syscalls::error();
    dir->close();

    strcpy(process->cwd, path);
    return 0;
}


SYSCALL(rename) {
    PROCESS
    RESOLVE_PATH(oldpath, regs->rdi)
    RESOLVE_PATH(newpath, regs->rsi)
    
    STRACE("rename(%s, %s)", oldpath, newpath);

    VFS::get()->rename(oldpath, newpath);

    if (haserr())
        return Syscalls::error();

    return 0;
}


SYSCALL(unlink) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)
    
    STRACE("unlink(%s)", path);

    VFS::get()->unlink(path);
    if (haserr())
        return Syscalls::error();

    return 0;
}


SYSCALL(readlink) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)
    
    STRACE("readlink(%s)", path);

    seterr(EINVAL);
    return Syscalls::error();
}


SYSCALL(gettimeofday) {
    auto tv = (struct timeval*)regs->rdi;
    auto tz = (struct timezone*)regs->rsi;
    STRACE("gettimeofday(0x%lx, 0x%lx)", tv, tz);

    tv->tv_sec = CMOS::get()->readTime();
    tv->tv_usec = 0;
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;

    return 0;
}


SYSCALL(sysinfo) {
    auto info = (struct sysinfo*)regs->rdi;

    STRACE("sysinfo(0x%lx)", info);

    info->totalram = FrameAlloc::get()->getTotal();
    info->freeram = info->totalram - FrameAlloc::get()->getAllocated();
    info->sharedram = KCFG_LOW_IDENTITY_PAGING_LENGTH + KCFG_HIGH_IDENTITY_PAGING_LENGTH + KCFG_KERNEL_HEAP_SIZE;
    info->sharedram /= KCFG_PAGE_SIZE;
    info->totalswap = 0;
    info->freeswap = 0;
    info->bufferram = 0;
    info->loads[0] = 0;
    info->loads[1] = 0;
    info->loads[2] = 0;
    info->totalhigh = 0;
    info->freehigh = 0;
    info->mem_unit = KCFG_PAGE_SIZE;
    info->uptime = Scheduler::get()->getUptime();
    info->procs = Scheduler::get()->processes.count();

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
    STRACE("pgid = %i", process->pgid);
    STRACE("ppid = %i", process->ppid);
    STRACE("pid = %i", process->pid);
    return process->pgid;

    //Process* p = Scheduler::get()->getProcess(pid);
    //if (haserr())
        //return Syscalls::error();
    //return p->pgid;
}


SYSCALL(arch_prctl) {
    //PROCESS
    
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


SYSCALL(sync) {
    STRACE("sync()");
    WARN_STUB("sync()")
    return 0;
}


SYSCALL(reboot) {
    auto cmd = regs->rdx;    
    
    STRACE("reboot(0x%lx)", cmd);

    if (cmd == RB_AUTOBOOT)
        PM::reboot();
    else if (cmd == RB_POWER_OFF)
        PM::shutdown();
    else 
        seterr(EINVAL);

    return Syscalls::error();
}


SYSCALL(time) {
    auto timeptr = (time_t*)regs->rdi;    
    time_t timev = 0;
    
    STRACE("time(0x%lx)", timeptr);
    WARN_STUB("time");

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
    char            d_name[256];
};

SYSCALL(getdents64) { 
    PROCESS
 
    auto fd = regs->rdi;    
    auto buf = (struct kernel_dirent64*)regs->rsi;
    auto sz = regs->rdx; // TODO take in account

    STRACE("getdents64(%u, 0x%lx, %u)", fd, buf, sz);

    auto dir = (Directory*)process->files[fd];
    uint64_t count = 0, num = 0;

    dirent* de;
    while ((de = dir->read())) {
        num++;
        buf->d_ino = de->d_ino + 5;
        buf->d_reclen = 256;// strlen(de->d_name) + 2;

        if ((count + buf->d_reclen) > sz)
            return num;

        buf->d_off = (count += buf->d_reclen);
        strcpy(buf->d_name, de->d_name);
        ///*(char*)(buf + buf->d_reclen - 1) = de->d_type;
        buf->d_type = de->d_type;
        buf += buf->d_reclen;
        
        return 1;
        //return count;
    }

    return num;
}


SYSCALL(utimes) {
    PROCESS
    RESOLVE_PATH(path, regs->rdi)
    auto times = (struct utimbuf*)regs->rsi;

    STRACE("utimes(%s, 0x%lx)", path, times);
    WARN_STUB("utimes");

    auto f = VFS::get()->open(path, O_RDONLY);
    if (haserr())
        return Syscalls::error();
    
    f->close();
    delete f;

    return 0;
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
    syscalls[0x07] = sys_poll;
    syscalls[0x08] = sys_lseek;
    syscalls[0x09] = sys_mmap;
    syscalls[0x0b] = sys_munmap;
    syscalls[0x0c] = sys_brk;
    syscalls[0x0d] = sys_rt_sigaction;
    syscalls[0x0e] = sys_sigprocmask;
    syscalls[0x10] = sys_ioctl;
    syscalls[0x14] = sys_writev;
    syscalls[0x15] = sys_access;
    syscalls[0x16] = sys_pipe;
    syscalls[0x20] = sys_dup;
    syscalls[0x21] = sys_dup2;
    syscalls[0x23] = sys_nanosleep;
    syscalls[0x27] = sys_getpid;
    syscalls[0x39] = sys_fork;
    syscalls[0x3a] = sys_vfork;
    syscalls[0x3b] = sys_execve;
    syscalls[0x3c] = sys_exit;
    syscalls[0x3d] = sys_wait4;
    syscalls[0x3e] = sys_kill;
    syscalls[0x3f] = sys_uname;
    syscalls[0x48] = sys_fcntl;
    syscalls[0x49] = sys_flock;
    syscalls[0x4f] = sys_getcwd;
    syscalls[0x50] = sys_chdir;
    syscalls[0x52] = sys_rename;
    syscalls[0x57] = sys_unlink;
    syscalls[0x59] = sys_readlink;
    syscalls[0x60] = sys_gettimeofday;
    syscalls[0x63] = sys_sysinfo;
    syscalls[0x66] = sys_getuid;
    syscalls[0x68] = sys_getuid; // getgid
    syscalls[0x6b] = sys_getuid; // geteuid
    syscalls[0x6c] = sys_getuid; // getegid
    syscalls[0x6d] = sys_setpgid; 
    syscalls[0x6f] = sys_getpgrp;
    syscalls[0x9e] = sys_arch_prctl;
    syscalls[0xa2] = sys_sync;
    syscalls[0xa9] = sys_reboot;
    syscalls[0xc9] = sys_time;
    syscalls[0xd9] = sys_getdents64;
    syscalls[0xeb] = sys_utimes;
    syscalls[0x6e] = sys_getppid;
}


extern "C" uint64_t _syscall_handler(syscall_regs_t* regs) {
    uint64_t result = 0;

    Scheduler::get()->pause();
    geterr(); // drop errors

    if (syscalls[regs->id]) {
        __strace_in_progress = false;
        result = syscalls[regs->id](regs);
        if (__strace_in_progress) {
            STRACE(" == %lx", result);
        }
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
