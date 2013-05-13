#include <core/Debug.h>
#include <core/Scheduler.h>
#include <memory/AddressSpace.h>
#include <core/Process.h>
#include <core/Thread.h>
#include <core/Wait.h>
#include <kutil.h>
#include <hardware/keyboard/Keyboard.h>


Message Debug::MSG_DUMP_REGISTERS("dump-registers");
Message Debug::MSG_DUMP_TASKS("dump-tasks");
Message Debug::MSG_DUMP_ADDRESS_SPACE("dump-address-space");


bool Debug::tracingOn = false;


static void onKeyboardEvent(keyboard_event_t* e) {
    if ((e->mods & 1) && e->scancode == 0xbb)
        Debug::MSG_DUMP_TASKS.post(NULL);
    if ((e->mods & 1) && e->scancode == 0xbc)
        Memory::log();
    if ((e->mods & 1) && e->scancode == 0xbd)
        Debug::MSG_DUMP_ADDRESS_SPACE.post(NULL);
    if ((e->mods & 1) && e->scancode == 0xbe)
        Debug::tracingOn = !Debug::tracingOn;
}

void Debug::init() {
    MSG_DUMP_REGISTERS.registerConsumer((MessageConsumer)&Debug::onDumpRegisters);
    MSG_DUMP_TASKS.registerConsumer((MessageConsumer)&Debug::onDumpTasks);
    MSG_DUMP_ADDRESS_SPACE.registerConsumer((MessageConsumer)&Debug::onDumpAddressSpace);
    Keyboard::MSG_KEYBOARD_EVENT.registerConsumer((MessageConsumer)&onKeyboardEvent);
}

void Debug::onDumpRegisters(isrq_registers_t* regs) {
    klog('w', "Register dump");
    klog('w', "RIP: %16lx   RSP: %16lx", regs->rip, regs->rsp);
    klog('w', "RDI: %16lx   RSI: %16lx", regs->rdi, regs->rsi);
    klog('w', "RAX: %16lx   RBX: %16lx", regs->rax, regs->rbx);
    klog('w', "RCX: %16lx   RDX: %16lx", regs->rcx, regs->rdx);
    klog_flush();
}

void Debug::onDumpTasks(void*) {
    klog('i', "");
    klog('i', "Tasks: -------------------------------------------------");
    klog('i', "Scheduler %s", Scheduler::get()->active ? "active" : "OFF");
    for (Process* p : Scheduler::get()->processes) {
        klog('s', "");
        klog('s', "[PID %3i] %s", p->pid, p->name);
        for (Thread* t : p->threads) {
            char* st = NULL;
            if (t->activeWait) {
                #define CHECK_WAIT(x) if (t->activeWait->type == x) st = #x;
                CHECK_WAIT(WAIT_FOREVER);
                CHECK_WAIT(WAIT_FOR_DELAY);
                CHECK_WAIT(WAIT_FOR_CHILD);
                CHECK_WAIT(WAIT_FOR_FILE);
            } else 
                st = "running";
            klog('i', " - TID %3i %10s | %15s | %4i cycles", 
                t->id, 
                t->name, 
                st,
                t->cycles
            );
        }
    }
    klog('i', "");
    klog_flush();
}

void Debug::onDumpAddressSpace(void* regs) {
    AddressSpace::current->dump();
    klog_flush();
}
