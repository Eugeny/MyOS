#include <lang/lang.h>
#include <tty/PhysicalTerminalManager.h>

#define WIDTH 80
#define HEIGHT 25

void PhysicalTerminalManager::init(int terminalCount) {
    count = terminalCount;
    terminals = new Terminal*[terminalCount];

    for (int i = 0; i < terminalCount; i++) {
        terminals[i] = new Terminal(WIDTH, HEIGHT);
    }

    switchTo(0);
    //klog_init(terminals[0]);
}

void PhysicalTerminalManager::switchTo(int a) {
    activeIdx = a;
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