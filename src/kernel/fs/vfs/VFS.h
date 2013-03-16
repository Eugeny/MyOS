#ifndef FS_VFS_VFS_H
#define FS_VFS_VFS_H

#include <fs/FS.h>
#include <lang/Pool.h>
#include <lang/Singleton.h>
#include <fs/File.h>
#include <fs/Directory.h>


struct mount_t {
    char path[255];
    FS* fs;
};

struct vfs_lookup_t {
    bool found;
    FS* fs;
    char path[255];
};


class VFS : public FS, public Singleton<VFS> {
public:
    void mount(char* point, FS* fs);
    vfs_lookup_t lookup(char* path);
    virtual StreamFile* open(char* path, int flags);
    virtual Directory* opendir(char* path);
    virtual void rename(char* opath, char* npath);
    virtual void unlink(char* path);
private:
    Pool<mount_t*, 1024> mounts;
};  


#endif