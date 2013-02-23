#include "kutil.h"

#include <stdio.h>
#include <string.h>

#include "alloc/malloc.h"


void sout(const char* str) {
    static int line = 0;

    char *vram = (char *)0xb8000;
    for (unsigned int i = 0; i < strlen(str); i++) {
        *(vram + i * 2 + 160 * line) = str[i];
    }
    line++;
}

void ktrace(const char* file, int line) {
    ktrace(file, line, "tracepoint");
}

void ktrace(const char* file, int line, char* msg) {
    char buffer[1024];
    sprintf(buffer, "%s:%i : %s", file, line, msg);
    sout(buffer);
}


//extern "C" void* __real_malloc(int c);
