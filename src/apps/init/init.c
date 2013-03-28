#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <sys/reboot.h>


#define SHELL "sh"
#define SHELLPATH "/bin/sh"

#define TASK_SHUTDOWN 1
#define TASK_REBOOT 2


static int task = 0;

void initiate_shutdown() {
    printf("\n * Shutting down\n");
    printf(" * Sending SIGTERM to all processes\n");
    kill(-1, SIGTERM);
    sleep(2);
    kill(-1, SIGKILL);

    printf(" * Waiting for all processes to terminate");
    while (1) {
        struct sysinfo sinfo;
        sysinfo(&sinfo);

        printf(".");
        fflush(NULL);

        printf("%i ", sinfo.procs);
        if (sinfo.procs == 2)
            break;

        sleep(1);
    }

    printf("\n");
    printf(" * All processes terminated\n");
    sleep(1);

    if (task == TASK_REBOOT)
        reboot(RB_AUTOBOOT);
    if (task == TASK_SHUTDOWN)
        reboot(RB_POWER_OFF);
}



void sig_handler(int sig) {
    if (sig == SIGTERM)
        task = TASK_REBOOT;
    if (sig == SIGUSR2)
        task = TASK_SHUTDOWN;
}


int main(int argc, char** argv) {
    fcntl(0, F_SETFD, 040000);
    fcntl(1, F_SETFD, 040000);
    fcntl(2, F_SETFD, 040000);

    printf("-----------------------------------------------------------------------------------------\n");
    printf("MyOS %s\n", argv[0]);

    const char* shell_args[] = { SHELL, NULL };

    const char* env[] = { 
        "HOME=/root",
        "PATH=/bin",
        "USER=root",
        "TERM=vt102",
        NULL 
    };

    chdir("/root");
    
    printf(" * Starting shell: %s\n", SHELLPATH);

    if (fork() == 0) {
        execve(SHELLPATH, shell_args, env);
    }
    
    printf("-----------------------------------------------------------------------------------------\n");

    signal(SIGTERM, &sig_handler);
    signal(SIGUSR2, &sig_handler);

    while (task == 0) {
        sleep(1);
    }

    if (task == TASK_SHUTDOWN || task == TASK_REBOOT)
        initiate_shutdown();
}
