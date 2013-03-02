#include <syscall/Syscalls.h>
#include <kutil.h>

extern "C" void _syscall_init();

typedef uint64_t (*syscall) (syscall_regs_t*);

static syscall syscalls[255];

#define SYSCALL(name) uint64_t sys_ ## name (syscall_regs_t* regs) 

#define STRACE(format, args...) {klog('t', format, ## args );klog_flush();}


//-----------------------------------
//-----------------------------------

#include <sys/utsname.h>
#include <string.h>

SYSCALL(open) {
    auto path = (char*)regs->rdi;    
    auto flags = regs->rsi;
    auto mode = regs->rdx;

    STRACE("open('%s', 0x%x, 0x%x)", path, flags, mode);
    
    return 0;
}

SYSCALL(uname) {
    auto buf = (struct utsname*)regs->rdi;
    
    STRACE("uname(%lx)", buf);

    strcpy(buf->sysname, "MyOS");
    strcpy(buf->nodename, "Test");
    strcpy(buf->release, "1.0");
    strcpy(buf->version, "1");
    strcpy(buf->machine, "x86-64");
}


//-----------------------------------
//-----------------------------------

void Syscalls::init() {
    _syscall_init();
    for (int i = 0; i < 255; i++)
        syscalls[i] = NULL;

    syscalls[0x02] = sys_open;
    syscalls[0x3f] = sys_uname;
}


extern "C" uint64_t _syscall_handler(syscall_regs_t* regs) {
    uint64_t result = 0;
    if (syscalls[regs->id]) {
        result = syscalls[regs->id](regs);
        klog('t', " = %lx", result);
        klog_flush();
    } else {
        klog('w', "Unknown syscall 0x%lx", regs->id);
        klog_flush();
    }
    for(int i =0;i<10000000;i++);
    return result;
}