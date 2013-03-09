#include <core/Debug.h>
#include <core/Scheduler.h>
#include <memory/AddressSpace.h>
#include <core/Process.h>
#include <core/Thread.h>
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
    klog('i', "Tasks: -------------------------------------------------");
    klog('i', "Scheduler %s", Scheduler::get()->active ? "active" : "OFF");
    for (Process* p : Scheduler::get()->processes) {
        klog('i', " - %3i %s", p->pid, p->name);
        for (Thread* t : p->threads) {
            uint64_t su = (uint64_t)t->stackBottom - t->state.regs.rsp;
            su = su * 100 / (t->stackSize + 1);
            klog('d', "      - %3i %30s | %s | %4i ticks | %i%% stack used of %lx kb", 
                t->id, 
                t->name, 
                t->activeWait ? "waiting" : "running",
                t->cycles,
                su,
                t->stackSize / 1024 
            );
        }
    }
    klog_flush();
}

void Debug::onDumpAddressSpace(void* regs) {
    AddressSpace::current->dump();
    klog_flush();
}
