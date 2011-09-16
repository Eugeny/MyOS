
extern int main(); //, char **environ);

extern char __bss_start, _end; // BSS should be the last think before _end

// XXX: environment
char *__env[1] = { 0 };
char **environ = __env;

_start(int argc, char **argv) {
  char *i;

  // zero BSS
  for(i = &__bss_start; i < &_end; i++){
    *i = 0;
  }


  // XXX: get argc and argv

  exit(main(argc,argv, __env));
}
