#include <hardware/pm.h>
#include <hardware/io.h>
#include <memory/AddressSpace.h>
#include <core/CPU.h>
#include <kutil.h>

extern "C" void ApmPoweroff();

void PM::reboot() {
    klog('w', "Rebooting");
    outb(0x64, 0xfe);
    CPU::halt();
}

void PM::shutdown() {
    klog('w', "Power off");
    ApmPoweroff();
}

void PM::halt() {
    klog('w', "System halted");
    klog_flush();
    CPU::CLI();
    CPU::halt();
}