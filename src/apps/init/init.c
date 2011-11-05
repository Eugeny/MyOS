#include <stdio.h>
#include <unistd.h>

int main() {
    printf("-------------------------------------------------------------------------------\n");
    printf("> MyOS init\n");
    printf("> Starting shells\n");
    exec("/bin/sh", "/dev/tty0");
    exec("/bin/sh", "/dev/tty1");
    exec("/bin/sh", "/dev/tty2");
    exec("/bin/sh", "/dev/tty3");
    exec("/bin/sh", "/dev/tty4");
}
