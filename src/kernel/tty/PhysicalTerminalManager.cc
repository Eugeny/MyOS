#include <lang/lang.h>
#include <tty/PhysicalTerminalManager.h>


#define WIDTH 80
#define HEIGHT 25


static void onKeyboardEvent(keyboard_event_t* e) {
    PhysicalTerminalManager::get()->dispatchKey(e);
}

void PhysicalTerminalManager::init(int terminalCount) {
    count = terminalCount;
    terminals = new Terminal*[terminalCount];
    ptys = new PTY*[terminalCount];

    for (int i = 0; i < terminalCount; i++) {
        terminals[i] = new Terminal(WIDTH, HEIGHT);
        ptys[i] = new PTY();
        terminals[i]->pty = ptys[i]->openMaster();
    }

    switchTo(0);
    Keyboard::MSG_KEYBOARD_EVENT.registerConsumer((MessageConsumer)&onKeyboardEvent);
}

void PhysicalTerminalManager::switchTo(int a) {
    activeIdx = a;
    getActiveTerminal()->makeDirty();
}

void PhysicalTerminalManager::render() {
    getActiveTerminal()->render();
}

Terminal* PhysicalTerminalManager::getActiveTerminal() {
    return getTerminal(activeIdx);
}

Terminal* PhysicalTerminalManager::getTerminal(int idx) {
    return terminals[idx];
}

int PhysicalTerminalManager::getTerminalCount() {
    return count;
}

PTYSlave* PhysicalTerminalManager::openPTY(int idx) {
    return ptys[idx]->openSlave();
}

PTY* PhysicalTerminalManager::getPTY(int idx) {
    return ptys[idx];
}

void PhysicalTerminalManager::dispatchKey(keyboard_event_t* event) {
    getActiveTerminal()->processKey(event->mods, event->scancode);
}
