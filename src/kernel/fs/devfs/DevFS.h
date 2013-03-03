#ifndef FS_DEVFS_DEVFS_H
#define FS_DEVFS_DEVFS_H

#include <fs/FS.h>
#include <fs/File.h>
#include <fs/Directory.h>


class DevFS : public FS {
public:
    DevFS();
    virtual File* open(char* path, int flags);
    virtual Directory* opendir(char* path);
};  


#endif