#ifndef TTY_TERMINAL_H
#define TTY_TERMINAL_H

#define WIDTH  80
#define HEIGHT 25

#include <rote.h>

class Terminal {
public:
    Terminal(int w, int h);
    void write(const char* data);
    void write(const char* buf, int offset, int len);
    void render();
    void makeDirty();
private:
    int         width, height;
    bool        dirty;
    RoteTerm*   terminal;
};
#endif
