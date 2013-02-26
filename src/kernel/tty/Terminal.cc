#include <lang/lang.h>
#include <tty/Terminal.h>
#include <string.h>

#include <hardware/vga/vga.h>


static char keymap[] = { 
//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x00 - 0x0f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x10 - 0x1f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x20 - 0x2f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x30 - 0x3f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x40 - 0x4f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x50 - 0x5f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x60 - 0x6f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x70 - 0x7f
    0  , 0  , '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\b','\t', // 0x80 - 0x8f
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']','\n', 0  , 'a', 's', // 0x90 - 0x9f
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0  , '`','\'','\\', 'z', 'x', 'c', 'v', // 0xa0 - 0xaf
    'b', 'n', 'm', ',', '.', '/', 0  , 0  , ' ', ' ', 0  , 0  , 0  , 0  , 0  , 0  , // 0xb0 - 0xbf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xc0 - 0xcf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xd0 - 0xdf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xe0 - 0xef
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xf0 - 0xff
};


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

void Terminal::processKey(uint64_t mods, uint64_t scancode) {
    char* map = keymap;
    if (map[scancode])
        write(&map[scancode], 0, 1);
}
