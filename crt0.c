#include <stdio.h>
#include <unistd.h>
#include <asm/prctl.h>
#include <sys/prctl.h>



extern int main(int argc, char** argv, char** env);

void _start(int argc, char** argv, char** env) {
    void* cbrk = brk(0);
  //  brk(cbrk + 0x2000);
//    arch_prctl(ARCH_SET_FS, cbrk);

    int exitcode = main(argc, argv, env);
    printf("EXIT=%i\n", exitcode);
    for(;;);
}
