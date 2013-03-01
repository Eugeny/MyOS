#ifndef TTY_TERMINAL_H
#define TTY_TERMINAL_H

#define WIDTH  80
#define HEIGHT 25

#include <rote.h>
#include <fs/devfs/PTY.h>


class Terminal {
public:
    Terminal(int w, int h);
    void write(const char* data);
    void write(const char* buf, int offset, int len);
    void render();
    void makeDirty();
    void processKey(uint64_t mods, uint64_t scancode);
    PTYMaster* pty;
private:
    int         width, height;
    bool        dirty;
    RoteTerm*   terminal;
};
#endif
