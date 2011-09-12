#ifndef VFS_FS_H
#define VFS_FS_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <io/FileObject.h>
#include <vfs/Stat.h>

class FS {
public:
    virtual Stat* stat(char* path);
    virtual LinkedList<char*>* listFiles(char* path) { return NULL; }
    virtual FileObject* open(char* path, int mode) { return NULL; }
};

#endif
