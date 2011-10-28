extern int main(int,char**,char**env);

extern char __bss, _end;

char *__env[1] = { 0 };
char **environ = __env;


#include <stdio.h>

int _myos_start(int argc, char **argv) {
    char *i;
    //FILE* ff = fopen("/dev/tty0","r");
    //fprintf(ff,"fdopen!");
    //close(ff);
    //FILE* f = fdopen(0,"r");
    //fprintf(f,"fdopen!");
    //return 0;
    //printf("STDIN");

    for(i = &__bss; i < &_end; i++){
        *i = 0;
    }

    int code = main(argc,argv, __env);
    __exit(code);
}
