#include "terminal.h"
#include "kutils.h"


Terminal::Terminal() {
    reset();
}    
 
void Terminal::reset() {   
    _cursorX = 0;	
    _cursorY = 0;
    _cursorVisible = TRUE;
    _attr = 0x7;
    _buffer = (direct)?((char *)0xb8000):buffer;
    dirty = 0;
}

void Terminal::write(char* s) {
    while (*s) {
        if (*s == '\n') {
            newLine();
        } else {
            setAttr(_cursorX, _cursorY, _attr);
            setCh(_cursorX, _cursorY, *s);
            _cursorX++;
        }
        s++;
    }
    dirty = 1;
}

void Terminal::newLine() {
    _cursorX = 0;
    _cursorY++;
    if (_cursorY == HEIGHT) {
        _cursorY--;
        scroll(1);
    }
}

void Terminal::setCh(int x, int y, char ch) {
    _buffer[getOffset(x,y)] = ch;
}

void Terminal::setAttr(int x, int y, u8int attr) {
    _buffer[getOffset(x,y)+1] = attr;
}

void Terminal::scroll(int dy) {
    memcpy((void*)((int)_buffer), (void*)((int)_buffer+WIDTH*2), WIDTH*2*(HEIGHT-dy));
    memset((void*)((int)_buffer+WIDTH*2*(HEIGHT-dy)), 0, WIDTH*2);
}

void Terminal::draw() {
    dirty = 0;
    if (direct) return;
    unsigned char *vram = (unsigned char *)0xb8000;
    memcpy(vram, _buffer, WIDTH*HEIGHT*2);
}

inline int Terminal::getOffset(int x, int y) {
    return (WIDTH*y+x)*2;
}
