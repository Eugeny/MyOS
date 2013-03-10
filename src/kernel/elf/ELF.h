#ifndef ELF_ELF_H
#define ELF_ELF_H

#include <lang/lang.h>
#include <core/Process.h>
#include <core/Thread.h>


class ELF {
public:
    ELF();
    ~ELF();
    void loadFromFile(char* path);
    void loadIntoProcess(Process* p);
    uint64_t getEntryPoint();
    Thread* startMainThread(Process* p, char** argv, char** envp);

private:
    char exeName[256];
    uint8_t* data;
};


#endif 

