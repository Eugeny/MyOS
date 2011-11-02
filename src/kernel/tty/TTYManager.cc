#include <tty/TTYManager.h>
#include <hardware/Keyboard.h>
#include <kutils.h>

char keymap[] = { 
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
    'b', 'n', 'm', ',', '.', '/', 0  , 0  , ' ', 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xb0 - 0xbf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xc0 - 0xcf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xd0 - 0xdf
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xe0 - 0xef
    0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , // 0xf0 - 0xff
};

void TTYManager::init(int ttyc) {
    ttyCount = ttyc;
    ttys = new TTY*[ttyc];
    terminals = new Terminal*[ttyc];
    for (int i = 0; i < ttyc; i++) {
        terminals[i] = new Terminal(WIDTH, HEIGHT-1);
        terminals[i]->top = 1;
        ttys[i] = new TTY(terminals[i]);
    }
    switchActive(0);
    klog_init(terminals[0]);

    bgTerm = new Terminal(WIDTH, 1);
}

void TTYManager::switchActive(int a) {
    activeIdx = a;
    activeTerminal = terminals[a];
    activeTerminal->dirty = true;
}

void TTYManager::draw() {
    bgTerm->setAttr(0x8F);
    bgTerm->goTo(0,0);
    bgTerm->write("Term");
    for (int i = 0; i < WIDTH; i++)
        bgTerm->setAttr(i, 0, 0x8F);
    for (int i = 0; i < ttyCount; i++) {
        bgTerm->setCh(6+i*3, 0, '0'+i);
    }
    bgTerm->setAttr(activeIdx*3+5, 0, 0x0F);
    bgTerm->setAttr(activeIdx*3+6, 0, 0x0F);
    bgTerm->setAttr(activeIdx*3+7, 0, 0x0F);

    bgTerm->draw();
    //if (activeTerminal->dirty)
        activeTerminal->draw();
}

TTY* TTYManager::getTTY(int idx) {
    return ttys[idx];
}

int TTYManager::getTTYCount() {
    return ttyCount;
}

Terminal* TTYManager::getStatusBar() {
    return bgTerm;
}

void TTYManager::processKey(u32int mod, u32int code) {
    if (mod == KBD_MOD_CTRL && code > 0x81 && code <= 0x82 + (u32int)ttyCount) {
        switchActive(code - 0x82);
    }

    if (mod == 0)
        if (keymap[code] != 0) {
            getTTY(activeIdx)->sendInputByte(keymap[code]);
            //DEBUG(to_hex(code));
        }
}
