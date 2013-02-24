#include <lang/lang.h>
#include <tty/Terminal.h>
#include <string.h>

#include <hardware/vga/vga.h>


Terminal::Terminal(int w, int h) {
    width = w;
    height = h;
    dirty = true;
    terminal = rote_vt_create(h, w);
    terminal->curattr = 0x70;
}


void Terminal::write(const char* s) {
    write(s, 0, strlen(s));
}

void Terminal::write(const char* buf, int offset, int len) {
    rote_vt_inject(terminal, (buf + offset), len);
    //render(); // TODO!
}

void Terminal::makeDirty() {
    dirty = true;
}

uint8_t COLORMAP[16] = {
    0, 4, 2, 6, 1, 5, 3, 7,
    8,12,10,14, 9,13,11,15,
};

void Terminal::render() {
    unsigned char *vram = (unsigned char *)0xb8000;
    uint8_t* cell = vram;
    for (int y = 0; y < height; y++) {
        if (0 && !(terminal->line_dirty[y]) && !dirty) {
            cell += width * 2;
            continue;
        }
        terminal->line_dirty[y] = false;
        for (int x = 0; x < width; x++) {
            *(cell++) = terminal->cells[y][x].ch;
            
            uint8_t attr = 0;
            uint8_t rote_attr = terminal->cells[y][x].attr;
            attr += COLORMAP[ROTE_ATTR_BG(rote_attr)] << 4;
            attr += COLORMAP[ROTE_ATTR_XFG(rote_attr)];

            *(cell++) = attr;
        }
    }
    vga_move_cursor(terminal->ccol, terminal->crow);
    dirty = false;
}