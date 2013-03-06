#include <core/Debug.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>
#include <hardware/keyboard/Keyboard.h>


Message Debug::MSG_DUMP_REGISTERS("dump-registers");
Message Debug::MSG_DUMP_TASKS("dump-tasks");


static void onKeyboardEvent(keyboard_event_t* e) {
    if ((e->mods & 1) && e->scancode == 0xbb)
        Debug::MSG_DUMP_TASKS.post(NULL);
}

void Debug::init() {
    MSG_DUMP_REGISTERS.registerConsumer((MessageConsumer)&Debug::onDumpRegisters);
    MSG_DUMP_TASKS.registerConsumer((MessageConsumer)&Debug::onDumpTasks);
    Keyboard::MSG_KEYBOARD_EVENT.registerConsumer((MessageConsumer)&onKeyboardEvent);
}

void Debug::onDumpRegisters(isrq_registers_t* regs) {
    klog('w', "Register dump");
    klog('w', "RIP: %016lx   RSP: %016lx", regs->rip, regs->rsp);
    klog('w', "RDI: %016lx   RSI: %016lx", regs->rdi, regs->rsi);
    klog('w', "RAX: %016lx   RBX: %016lx", regs->rax, regs->rbx);
    klog('w', "RCX: %016lx   RDX: %016lx", regs->rcx, regs->rdx);
    klog_flush();
}

void Debug::onDumpTasks(void*) {
    klog('i', "Tasks:");
    for (Process* p : Scheduler::get()->processes) {
        klog('i', " - %3i %s", p->pid, p->name);
        for (Thread* t : p->threads) {
            klog('d', "      - %3i %10s @ %16lx", t->id, t->name, t->state.regs.rip);
        }
    }
    klog_flush();
}