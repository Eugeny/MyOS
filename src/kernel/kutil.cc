#include "kutil.h"

#include <stdio.h>
#include <string.h>

#include "alloc/malloc.h"


void microtrace() {
    char *vram = (char *)0xb8000;
    *vram = '+';
}

void sout(const char* str) {
    static int line = 0;

    char *vram = (char *)0xb8000;
    for (unsigned int i = 0; i < strlen(str); i++) {
        *(vram + i * 2 + 160 * line) = str[i];
    }
    
    line++;
    line %=25;
}

void ktrace(const char* file, int line) {
    ktrace(file, line, "tracepoint");
}

void ktrace(const char* file, int line, const char* msg) {
    char buffer[1024];
    sprintf(buffer, "%s:%i : %s", file, line, msg);
    sout(buffer);
}

void ktracemem(const char* file, int line) {
    kheap_info_t mem = kmallinfo();
    char buffer[1024];
    sprintf(buffer, "%s:%i : %i/%i bytes", file, line, mem.used_bytes, mem.total_bytes);
    sout(buffer);   
}
