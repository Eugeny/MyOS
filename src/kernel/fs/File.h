#ifndef FS_FILE_H
#define FS_FILE_H

#include <lang/lang.h>


class File {
public:
    virtual void write(const void* buffer, uint64_t count) = 0;
    virtual uint64_t read(void* buffer, uint64_t count) = 0;
    virtual void close();
    virtual bool canRead() = 0;

    char* path;
};


#endif