#ifndef FS_FS_H
#define FS_FS_H

#include <lang/lang.h>
#include <fs/File.h>
#include <fs/Directory.h>
#include <sys/stat.h>


class FS {
public:
    virtual StreamFile* open(char* path, int flags) = 0;
    virtual Directory* opendir(char* path) = 0;
    virtual void unlink(char* path);
};

#endif