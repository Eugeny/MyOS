#ifndef TTY_TTYMANAGER_H
#define TTY_TTYMANAGER_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <tty/TTY.h>
#include <tty/Terminal.h>


class TTYManager : public Singleton<TTYManager> {
public:
    void init(int ttyc);
    void draw();
    void switchActive(int idx);
    TTY* getTTY(int idx);
    int  getTTYCount();
    Terminal* getStatusBar();
    void processKey(u32int mod, u32int code);
    Terminal* activeTerminal;
private:
    int ttyCount, activeIdx;
    TTY** ttys;
    Terminal** terminals;
    Terminal* bgTerm;
};

#endif
