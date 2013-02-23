#include <tty/TTY.h>
#include <kutils.h>


TTY::TTY(Terminal *t) {
    inputBufferLen = 0;
    terminal = t;
}

void TTY::write(char* buf, int pos, int count) {
    terminal->write(buf, pos, count);
}

int TTY::read(char* buf, int pos, int max) { //TODO blocking?
    while (inputBufferLen == 0);
    memcpy(buf + pos, inputBuffer, max);
    int tmp = inputBufferLen;
    inputBufferLen = 0;
    return tmp;
}

void TTY::sendInput(char* s) {
	writeString(s);
    while (*s)
        inputBuffer[inputBufferLen++] = *(s++);
}

void TTY::sendInputByte(char s) {
    inputBuffer[inputBufferLen++] = s;
    writeByte(s);
}
