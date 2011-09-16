#ifndef IO_FILEOBJECT_H
#define IO_FILEOBJECT_H

#include <util/cpp.h>

#define MODE_R 1
#define MODE_W 2
#define MODE_A 4
#define MODE_B 8



class FileObject {
public:
    virtual void write(char* buf, int pos, int count) {}
    virtual int  read(char* buf, int pos, int max) { return 0; }
    virtual void close() {}

    virtual void writeString(char* s);
    virtual void writeByte(char c);
};

#endif
