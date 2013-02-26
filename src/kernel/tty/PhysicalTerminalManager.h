#ifndef TTY_PHYSICALTERMINALMANAGER_H
#define TTY_PHYSICALTERMINALMANAGER_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <tty/Terminal.h>


class PhysicalTerminalManager : public Singleton<PhysicalTerminalManager> {
public:
    void init(int terminalCount);
    void render();
    void switchTo(int idx);
    Terminal* getTerminal(int idx);
    Terminal* getActiveTerminal();
    int  getTerminalCount();
    void dispatchKey(uint64_t mod, uint64_t code);
private:
    int count, activeIdx;
    Terminal** terminals;
};

#endif
