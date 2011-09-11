#include <tty/TTYManager.h>
#include <hardware/Keyboard.h>
#include <kutils.h>


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
    if (activeTerminal->dirty)
        activeTerminal->draw();
}

TTY* TTYManager::getTTY(int idx) {
    return ttys[idx];
}

Terminal* TTYManager::getStatusBar() {
    return bgTerm;
}

void TTYManager::processKey(u32int mod, u32int code) {
    if (mod == KBD_MOD_CTRL && code > 0x81 && code <= 0x82 + ttyCount) {
        switchActive(code - 0x82);
    }
}
