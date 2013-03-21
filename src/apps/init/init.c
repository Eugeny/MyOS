#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv) {
    printf("-----------------------------------------------------------------------------------------\n");
    printf("MyOS %s\n", argv[0]);
    printf("-----------------------------------------------------------------------------------------\n");

    const char* shell_args[] = { "sh", NULL };

    const char* env[] = { 
        "HOME=/root",
        "PATH=/bin",
        "USER=root",
        "TERM=vt102",
        NULL 
    };

    chdir("/root");
    
    if (fork() == 0) {
        execve("/bin/sh", shell_args, env);
    }

    for(;;);
}
