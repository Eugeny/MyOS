#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
    printf("Testing app running\n");

    if (strcmp(argv[1], "segfault") == 0)
        *(int*)(0x500000000000) = 1;

    if (strcmp(argv[1], "gpf") == 0)
        *(int*)(0x900000000000) = 1;

    printf("Valid commands are: gpf, segfault");
}
