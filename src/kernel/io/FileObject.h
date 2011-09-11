#ifndef IO_FILEOBJECT_H
#define IO_FILEOBJECT_H

#include <util/cpp.h>

class FileObject {
public:
    virtual void write(char* buf, int pos, int count) {}
    virtual int  read(char* buf, int pos, int max) { return 0; }

    virtual void writeString(char* s);
    virtual void writeByte(char c);
};

#endif
