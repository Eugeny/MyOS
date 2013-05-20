#include <lang/lang.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <tty/Terminal.h>
#include <string.h>
#include <kutil.h>
#include <signal.h>

#include <hardware/vga/VGA.h>
#include <ncurses.h>


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
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`','\'','\\', 'z', 'x', 'c', 'v', // 0xa0 - 0xaf
    'b', 'n', 'm', ',', '.', '/', 0  , 0  , ' ', ' ', 0  , 0  , 0  , 0  , 0  , 0  , // 0xb0 - 0xbf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xc0 - 0xcf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xd0 - 0xdf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xe0 - 0xef
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xf0 - 0xff
};

static char keymap_shifted[] = { 
//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x00 - 0x0f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x10 - 0x1f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x20 - 0x2f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x30 - 0x3f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x40 - 0x4f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x50 - 0x5f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x60 - 0x6f
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0x70 - 0x7f
    0  , 0  , '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+','\b','\t', // 0x80 - 0x8f
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}','\n', 0  , 'A', 'S', // 0x90 - 0x9f
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~','"', '|' , 'Z', 'X', 'C', 'V', // 0xa0 - 0xaf
    'B', 'N', 'M', '<', '>', '?', 0  , 0  , ' ', ' ', 0  , 0  , 0  , 0  , 0  , 0  , // 0xb0 - 0xbf
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
    pty = NULL;
}


void Terminal::write(const char* s) {
    write(s, 0, strlen(s));
}

void Terminal::write(const char* buf, int offset, int len) {
    rote_vt_inject(terminal, (buf + offset), len);
}

void Terminal::makeDirty() {
    dirty = true;
}

uint8_t COLORMAP[16] = {
    0, 4, 2, 6, 1, 5, 3, 7,
    8,12,10,14, 9,13,11,15,
};

void Terminal::render() {
    if (pty) {
        char buffer[1024];
        while (int c = pty->read(buffer, 1024)) {
            write(buffer, 0, c);
        }
    }

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

    VGA::moveCursor(terminal->ccol, terminal->crow);
    
    dirty = false;
}

void Terminal::processKey(uint64_t mods, uint64_t scancode) {
    char* map = (mods == 2) ? keymap_shifted : keymap;
    
    #define WRITE_ESCAPED(x) { pty->write("\033[", 2); pty->write(x, strlen(x)); return; }

    if (scancode == 0xcb) WRITE_ESCAPED("D");
    if (scancode == 0xcd) WRITE_ESCAPED("C");
    if (scancode == 0xc8) WRITE_ESCAPED("A");
    if (scancode == 0xd0) WRITE_ESCAPED("B");

    if (scancode == 0xc7) WRITE_ESCAPED("1~");
    if (scancode == 0xcf) WRITE_ESCAPED("4~");
    if (scancode == 0xc9) WRITE_ESCAPED("5~");
    if (scancode == 0xd1) WRITE_ESCAPED("6~");

    if (scancode == 0x81) {
        pty->write("\033", 1);
    }

    uint8_t mapped = map[scancode];

    if (mapped) {
        if (mods == 1 && mapped >= 'a' && mapped <= 'z') { // ctrl keys
            char buf = mapped - 64;
            pty->write(&buf, 1);
            klog('d', "Control key %c", mapped);

            for (auto p : Scheduler::get()->processes) {
                if (mapped == 'c') {
                    p->queueSignal(SIGINT);
                }
                if (mapped == 'z') {
                    p->queueSignal(SIGSTOP);
                }
            }
        } else {
            //write(&map[scancode], 0, 1);
            if (pty)
                pty->write(&map[scancode], 1);
        }
    } else
        klog('d', "Unknown key %lx/%lx", scancode, mods);
}
