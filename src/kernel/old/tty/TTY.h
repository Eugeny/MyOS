#ifndef TTY_TTY_H
#define TTY_TTY_H

#include <util/cpp.h>
#include <io/FileObject.h>
#include <tty/Terminal.h>

#define BUFFER (1024*20)

class TTY : public FileObject {
public:
    TTY(Terminal *t);
    virtual void write(char* buf, int pos, int count);
    virtual int  read(char* buf, int pos, int max);
    void sendInput(char* s);
    void sendInputByte(char s);
private:
    Terminal *terminal;
    int  inputBufferLen;
    char inputBuffer[BUFFER];
};

#endif
