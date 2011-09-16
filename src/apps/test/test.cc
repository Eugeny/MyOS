#include <syscall/Syscalls.h>

int main() {
    syscall_kprint("APP!");
    syscall_die();
}
