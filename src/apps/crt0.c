#include <stdio.h>
#include <unistd.h>

extern int main(int,char**,char**env);

extern char __bss, _end;

char *__env[1] = { 0 };
char **environ = __env;

_start(int argc, char **argv) {
    char *i;
    for(i = &__bss; i < &_end; i++){
        *i = 0;
    }

  _exit(main(argc,argv, __env));
}
