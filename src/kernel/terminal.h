#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

#define WIDTH  80
#define HEIGHT 25


class Terminal {
public:
    Terminal();
    void reset();
    void write(char* data);
    void draw();
    void setCh(int x, int y, char ch);
    void setAttr(int x, int y, u8int attr);
    char getCh(int x, int y);
    u8int getAttr(int x, int y);
    void scroll(int dy);
private:
    char _buffer[WIDTH*HEIGHT*2];
    int _cursorX, _cursorY;
    u8int _attr;
    bool _cursorVisible;
    
    int getOffset(int x, int y);
    void newLine();
};
#endif
