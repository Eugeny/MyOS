#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>


char* STOPFILE = "/clock-stop";

char* vmem = (char*)(0xb8000);


int file_exist (char *filename)
{
    struct stat buffer;   
    return (stat(filename, &buffer) == 0);
}


int main(int argc, char** argv) {
    char buffer[256];
    struct sysinfo info;
    
    if (argc == 2) {
        printf("Stopping background uptime clock\n");
        fclose(fopen(STOPFILE, "w"));
        usleep(1000000);
        unlink(STOPFILE);
        return 0;
    }


    printf("Starting background uptime clock\n");
    if (fork() == 0) {
        while (1) {
            sysinfo(&info);
            sprintf(buffer, "Uptime: %i s", info.uptime);
            for (int i = 0; i < strlen(buffer); i++)
                *(vmem + i*2) = buffer[i];


            if (file_exist(STOPFILE)) {
                unlink(STOPFILE);
                return 0;
            }
        }   
    }

    return 0;
}
