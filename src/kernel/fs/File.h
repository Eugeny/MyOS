#ifndef FS_FILE_H
#define FS_FILE_H

#include <lang/lang.h>


class FS;

class File {
public:
    virtual void write(void* buffer, uint64_t count) = 0;
    virtual uint64_t read(void* buffer, uint64_t count) = 0;
    virtual void close() = 0;

    FS* filesystem;
    char* path;
};


#endif