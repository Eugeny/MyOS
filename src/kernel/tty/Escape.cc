#include <tty/Escape.h>


const char* Escape::C_BLACK   = "\x1b[30m";
const char* Escape::C_RED     = "\x1b[31m";
const char* Escape::C_GREEN   = "\x1b[32m";
const char* Escape::C_YELLOW  = "\x1b[33m";
const char* Escape::C_BLUE    = "\x1b[34m";
const char* Escape::C_MAGENTA = "\x1b[35m";
const char* Escape::C_CYAN    = "\x1b[36m";
const char* Escape::C_B_GRAY   = "\x1b[37m";

const char* Escape::C_GRAY   = "\x1b[30;1m";
const char* Escape::C_B_RED     = "\x1b[31;1m";
const char* Escape::C_B_GREEN   = "\x1b[32;1m";
const char* Escape::C_B_YELLOW  = "\x1b[33;1m";
const char* Escape::C_B_BLUE    = "\x1b[34;1m";
const char* Escape::C_B_MAGENTA = "\x1b[35;1m";
const char* Escape::C_B_CYAN    = "\x1b[36;1m";
const char* Escape::C_B_WHITE   = "\x1b[37;1m";

const char* Escape::C_OFF = "\x1b[0m";