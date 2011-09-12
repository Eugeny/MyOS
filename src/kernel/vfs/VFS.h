#ifndef VFS_VFS_H
#define VFS_VFS_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <util/LinkedList.h>
#include <io/FileObject.h>
#include <vfs/FS.h>
#include <vfs/Stat.h>


class Mountpoint {
public:
    FS* fs;
    char* path;
};


class VFS : public Singleton<VFS> {
public:
    void     init();
    void     mount(FS* fs, char* path);
    LinkedList<char*>* listFiles(char* node);
    LinkedList<Mountpoint*>* mounts;
    Mountpoint* getFS(char* path, bool mounted);
    Stat* stat(char* path);
    FileObject* open(char* path, int mode);

    static LinkedList<char*>* splitPath(char* path);
};

#endif
