#include <stdio.h>
#include <unistd.h>

int main() {
printf("HELLO");
    FILE* f = fopen("/dev/tty0", "w");
kprint(__FILE__);
   fprintf("APP!\n", f);
kprint(__FILE__);
   fread(0,0 ,0, f);
kprint(__FILE__);
   fflush(f);
    for(;;);
}
