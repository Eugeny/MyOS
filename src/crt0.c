#include <stdio.h>
#include <unistd.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <stdlib.h>

extern int main(int argc, char** argv, char** env);

void _start(int argc, char** argv, char** env) {
    exit(main(argc, argv, env));
    for(;;);
}
