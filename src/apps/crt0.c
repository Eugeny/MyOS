extern int main(int,char**,char**env);

extern char __bss, _end;

char *__env[1] = { 0 };
char **environ = __env;


#include <stdio.h>

int _myos_start(int argc, char **argv) {
    char *i;

    for(i = &__bss; i < &_end; i++){
        *i = 0;
    }

    int code = main(argc,argv, __env);
    __exit(code);
}
