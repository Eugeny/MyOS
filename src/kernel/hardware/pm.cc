#include <hardware/pm.h>
#include <hardware/io.h>
#include <core/CPU.h>
#include <kutil.h>


void PM::reboot() {
    klog('w', "Rebooting");
    outb(0x64, 0xfe);
    CPU::halt();
}

void PM::shutdown() {

}