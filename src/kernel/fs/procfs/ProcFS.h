#ifndef FS_PROCFS_PROCFS_H
#define FS_PROCFS_PROCFS_H

#include <fs/FS.h>
#include <fs/File.h>
#include <fs/Directory.h>


class ProcFS : public FS {
public:
    ProcFS();
    virtual char* getName();
    virtual StreamFile* open(char* path, int flags);
    virtual Directory* opendir(char* path);
};  


#endif