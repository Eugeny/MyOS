#ifndef VFS_ROOTFS_H
#define VFS_ROOTFS_H

#include <util/cpp.h>
#include <vfs/FS.h>

class RootFS : public FS {
public:
    RootFS();
    virtual LinkedList<char*>* listFiles(char* node);
    virtual Stat* stat(char* path);
};

#endif
