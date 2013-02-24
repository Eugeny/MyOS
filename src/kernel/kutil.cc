#include "kutil.h"

#include <stdio.h>
#include <string.h>
#include <tty/Escape.h>
#include <tty/PhysicalTerminalManager.h>

#include "alloc/malloc.h"


bool __logging_terminal_ready = false;

void klog_init() {
    __logging_terminal_ready = true;
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

void klog(char type, const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    vsprintf(buffer, format, args);

    if (__logging_terminal_ready) {
        Terminal* t = PhysicalTerminalManager::get()->getActiveTerminal();    

        if (type == 't') {
            t->write(Escape::C_GRAY);
            t->write("TRACE");
        } else {
            t->write(Escape::C_CYAN);
            t->write("???  ");
        }
        t->write(" :: ");
        t->write(buffer);
        t->write(Escape::C_OFF);
        t->write("\n");
        t->render(); // force render ASAP
    } else {
        sout(buffer);
    }

    va_end(args);
}

void microtrace() {
    char *vram = (char *)0xb8000;
    *vram = '+';
}


void ktrace(const char* file, int line) {
    ktrace(file, line, "tracepoint");
}

void ktrace(const char* file, int line, const char* msg) {
    klog('t', "%s:%i : %s", file, line, msg);
}

void ktracemem(const char* file, int line) {
    kheap_info_t mem = kmallinfo();
    klog('t', "%s:%i : %i/%i bytes", file, line, mem.used_bytes, mem.total_bytes);   
}
