#ifndef ELF_ELF_H
#define ELF_ELF_H

#include <lang/lang.h>
#include <fs/File.h>
#include <core/Process.h>


class ELF {
public:
    ELF();
    void loadFromFile(StreamFile* f);
    void loadIntoProcess(Process* p);
    uint64_t getEntryPoint();
private:
    uint8_t* data;
};


#endif 

