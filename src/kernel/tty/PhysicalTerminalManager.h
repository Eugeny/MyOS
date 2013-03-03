#ifndef TTY_PHYSICALTERMINALMANAGER_H
#define TTY_PHYSICALTERMINALMANAGER_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <tty/Terminal.h>
#include <fs/devfs/PTY.h>
#include <hardware/keyboard/Keyboard.h>


class PhysicalTerminalManager : public Singleton<PhysicalTerminalManager> {
public:
    void init(int terminalCount);
    void render();
    void switchTo(int idx);
    Terminal* getTerminal(int idx);
    Terminal* getActiveTerminal();
    PTYSlave* openPTY(int idx);
    PTY* getPTY(int idx);
    int  getTerminalCount();
    void dispatchKey(keyboard_event_t*);
private:
    int count, activeIdx;
    Terminal** terminals;
    PTY** ptys;
};

#endif
