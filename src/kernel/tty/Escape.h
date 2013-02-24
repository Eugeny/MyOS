#ifndef TTY_ESCAPE_H
#define TTY_ESCAPE_H

class Escape {
public:
    static const char *C_RED;
    static const char *C_BLACK;
    static const char *C_GREEN;
    static const char *C_YELLOW;
    static const char *C_BLUE;
    static const char *C_MAGENTA;
    static const char *C_CYAN;
    static const char *C_WHITE;
    static const char *C_B_BLACK;
    static const char *C_B_GREEN;
    static const char *C_B_YELLOW;
    static const char *C_B_BLUE;
    static const char *C_B_MAGENTA;
    static const char *C_B_CYAN;
    static const char *C_B_WHITE;
    static const char *C_B_RED;

    static const char *C_OFF;
};

const char* Escape::C_BLACK   = "\x1b[30m";
const char* Escape::C_RED     = "\x1b[31m";
const char* Escape::C_GREEN   = "\x1b[32m";
const char* Escape::C_YELLOW  = "\x1b[33m";
const char* Escape::C_BLUE    = "\x1b[34m";
const char* Escape::C_MAGENTA = "\x1b[35m";
const char* Escape::C_CYAN    = "\x1b[36m";
const char* Escape::C_WHITE   = "\x1b[37m";

const char* Escape::C_B_BLACK   = "\x1b[30;1m";
const char* Escape::C_B_RED     = "\x1b[31;1m";
const char* Escape::C_B_GREEN   = "\x1b[32;1m";
const char* Escape::C_B_YELLOW  = "\x1b[33;1m";
const char* Escape::C_B_BLUE    = "\x1b[34;1m";
const char* Escape::C_B_MAGENTA = "\x1b[35;1m";
const char* Escape::C_B_CYAN    = "\x1b[36;1m";
const char* Escape::C_B_WHITE   = "\x1b[37;1m";

const char* Escape::C_OFF = "\x1b[0m";
#endif