#ifndef FS_FS_H
#define FS_FS_H

#include <lang/lang.h>
#include <fs/File.h>
#include <fs/Directory.h>

class FS {
public:
    virtual File* open(char* path, int flags) = 0;
    virtual Directory* opendir(char* path) = 0;
};

#endif