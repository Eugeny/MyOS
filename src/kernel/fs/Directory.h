#ifndef FS_DIRECTORY_H
#define FS_DIRECTORY_H

#include <lang/lang.h>
#include <dirent.h>


class FS;

class Directory {
public:
    virtual struct dirent* read() = 0;
    virtual void close() = 0;

    FS* filesystem;
    struct dirent currentEntry;
    char* path;
};


#endif