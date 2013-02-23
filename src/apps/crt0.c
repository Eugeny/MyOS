extern int main(int,char**,char**env);

extern char __bss_start, _end;

char *__env[1] = { 0 };
char **environ = __env;


#include <stdio.h>

int _myos_start(int argc, char **argv) {
    char *i;

    for(i = &__bss_start; i < &_end; i++){
        *i = 0;
    }

    int code = main(argc,argv, __env);
    return code;
    //__exit(code);
}
