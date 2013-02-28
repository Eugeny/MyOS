#include <lang/lang.h>
#include <core/MQ.h>
#include <tty/PhysicalTerminalManager.h>


#define WIDTH 80
#define HEIGHT 25


static void onKeyboardEvent(keyboard_event_t* e) {
    PhysicalTerminalManager::get()->dispatchKey(e);
}

void PhysicalTerminalManager::init(int terminalCount) {
    count = terminalCount;
    terminals = new Terminal*[terminalCount];

    for (int i = 0; i < terminalCount; i++) {
        terminals[i] = new Terminal(WIDTH, HEIGHT);
    }

    switchTo(0);
    MQ::registerConsumer(Keyboard::MSG_KEYBOARD_EVENT, (MessageConsumer)&onKeyboardEvent);
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

void PhysicalTerminalManager::dispatchKey(keyboard_event_t* event) {
    getActiveTerminal()->processKey(event->mods, event->scancode);
}
