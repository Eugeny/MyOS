#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv) {
    printf("-----------------------------------------------------------------------------------------\n");
    printf("MyOS %s.\n", argv[0]);
    printf("Starting shells...\n");

    const char* shell_args[] = { "busybox", "sh", NULL };

    if (fork() == 0) {
        execv("/bin/sh", shell_args);
    }

    for(;;);
}
