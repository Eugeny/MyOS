#ifndef TTY_TERMINAL_H
#define TTY_TERMINAL_H

#include <util/cpp.h>

#define WIDTH  80
#define HEIGHT 25


class Terminal {
public:
    Terminal(int w, int h);
    void reset();
    void write(char* data);
    void write(char* buf, int offset, int len);
    void draw();
    void setCh(int x, int y, char ch);
    void setAttr(int x, int y, u8int attr);
    void setAttr(u8int attr);
    void goTo(int x, int y);
    char getCh(int x, int y);
    u8int getAttr(int x, int y);
    void scroll(int dy);

    int dirty, left, top, width, height;
private:
    char buffer[WIDTH*HEIGHT*2] __attribute__ ((aligned (16)));
    char* _buffer;
    int _cursorX, _cursorY;
    u8int _attr;
    bool _cursorVisible;

    int getOffset(int x, int y);
    void newLine();
};
#endif
