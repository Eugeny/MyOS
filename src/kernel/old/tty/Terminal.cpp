#include <tty/Terminal.h>


Terminal::Terminal(int w, int h) {
    reset();
    width = w;
    height = h;
}

void Terminal::reset() {
    _cursorX = 0;
    _cursorY = 0;
    _cursorVisible = true;
    _attr = 0x7;
    _buffer = buffer;
    dirty = 0;
    left = 0;
    top = 0;
}


void Terminal::write(char* s) {
    write(s, 0, strlen(s));
}

void Terminal::write(char* buf, int offset, int len) {
    buf += offset;
    len -= offset;
    while (len--) {
        if (_cursorX == width)
            newLine();
        if (*buf == '\n') {
            newLine();
        } else {
            setAttr(_cursorX, _cursorY, _attr);
            setCh(_cursorX, _cursorY, *buf);
            _cursorX++;
        }
        buf++;
    }
    dirty = 1;
}

void Terminal::newLine() {
    _cursorX = 0;
    _cursorY++;
    if (_cursorY == height) {
        _cursorY--;
        scroll(1);
    }
}

void Terminal::setCh(int x, int y, char ch) {
    _buffer[getOffset(x,y)] = ch;
}

void Terminal::setAttr(u8int attr) {
    _attr = attr;
}

void Terminal::setAttr(int x, int y, u8int attr) {
    _buffer[getOffset(x,y)+1] = attr;
}

void Terminal::goTo(int x, int y) {
    _cursorX = x;
    _cursorY = y;
}

void Terminal::scroll(int dy) {
    memcpy((void*)((int)_buffer), (void*)((int)_buffer+width*2), width*2*(height-dy));
    memset((void*)((int)_buffer+width*2*(height-dy)), 0, width*2);
}

void Terminal::draw() {
    dirty = 0;
    unsigned char *vram = (unsigned char *)0xb8000;
    for (int y = 0; y < height; y++)
        memcpy(vram + top*WIDTH*2 + y*width*2 + left*2, _buffer + y*width*2, width*2);
}

inline int Terminal::getOffset(int x, int y) {
    return (WIDTH*y+x)*2;
}
