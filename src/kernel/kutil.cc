#include "kutil.h"

#include <stdio.h>
#include <string.h>
#include <memory/FrameAlloc.h>
#include <tty/Escape.h>
#include <tty/PhysicalTerminalManager.h>

#include "alloc/malloc.h"


bool __logging_terminal_ready = false;
bool __logging_allowed = false;

void klog_init() {
    __logging_allowed = true;
}

void klog_init_terminal() {
    __logging_terminal_ready = true;
}



void __outputhex(uint64_t h, int offset) {
    char chr[] = "0123456789abcdef";

    uint64_t cr0 =(uint64_t) h;
    for (char c = 0; c < 8; c++) {
        *((char *)0xb8000 + offset * 2 - c * 2) = chr[cr0 % 16];
        *((char *)0xb8000 + offset * 2 - c * 2 + 1) = 0x0f;
        cr0 /=16;
    }
}

void __output(const char* s, int offset) {
    for (char c = 0; c < strlen(s); c++) {
        *((char *)0xb8000 + offset * 2 + c * 2) = s[c];
        *((char *)0xb8000 + offset * 2 + c * 2 + 1) = 0x0f;
    }
}

void sout(const char* str) {
    static int line = 0;

    char *vram = (char *)0xb8000;
//    for (unsigned int i = 0; i < 20; i++) {
    for (unsigned int i = 0; i < strlen(str); i++) {
        *(vram + i * 2 + 160 * line) = str[i];
    }
    
    line++;
    line %=25;
}

void klog(char type, const char* format, ...) {
    if (!__logging_allowed)
        return;
    va_list args;
    va_start(args, format);

    char buffer[1024];
    vsprintf(buffer, format, args);

    if (__logging_terminal_ready) {
        Terminal* t = PhysicalTerminalManager::get()->getActiveTerminal();    

        if (type == 't') {
            t->write(Escape::C_GRAY);
            t->write("TRACE");
        } else if (type == 'd') {
            t->write(Escape::C_B_GRAY);
            t->write("DEBUG");
        } else if (type == 'w') {
            t->write(Escape::C_B_YELLOW);
            t->write("WARN ");
        } else if (type == 'e') {
            t->write(Escape::C_B_RED);
            t->write("ERROR");
        } else if (type == 'i') {
            t->write(Escape::C_B_WHITE);
            t->write("INFO ");
        } else {
            t->write(Escape::C_CYAN);
            t->write("???  ");
        }
        t->write(" :: ");
        t->write(buffer);
        t->write(Escape::C_OFF);
        t->write("\n");

        #ifdef KCFG_LOG_FORCE_RENDER
            t->render(); // force render ASAP
        #endif
    } else {
        sout(buffer);
    }

    va_end(args);
}

void microtrace() {
    static char icons[] = "/-\\|";
    static int index = 0;
    index = (index + 1) % 4;
    char *vram = (char *)0xb809e;
    *vram = icons[index];
}


void ktrace(const char* file, int line) {
    ktrace(file, line, "tracepoint");
}

void ktrace(const char* file, int line, const char* msg) {
    klog('t', "%s:%i : %s", file, line, msg);
}

void ktracemem(const char* file, int line) {
    kheap_info_t mem = kmallinfo();
    klog('t', "Memory trace: %s:%i", file, line);   
    klog('t', "Heap:   %i/%i bytes", mem.used_bytes, mem.total_bytes);   
    klog('t', "Frames: %i/%i", FrameAlloc::get()->getAllocated(), FrameAlloc::get()->getTotal());   
}

void klog_flush() {
    PhysicalTerminalManager::get()->render();
}