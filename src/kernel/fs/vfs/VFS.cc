#include <fs/vfs/VFS.h>
#include <string.h>
#include <kutil.h>
#include <errno.h>


void VFS::mount(char* point, FS* fs) {
    auto m = new mount_t();
    strcpy(m->path, point);
    m->fs = fs;
    klog('i', " -- mounting %s at %s", fs->getName(), point);
    mounts.add(m);
}

vfs_lookup_t VFS::lookup(char* path) {
    vfs_lookup_t result;
    result.found = false;

    for (mount_t* m : mounts) {
        if (strstr(path, m->path) == path) {
            result.found = true;
            result.fs = m->fs;
            uint64_t offset = strlen(m->path);
            if (offset == 1) offset = 0; // fix root
            strcpy(result.path, (char*)((uint64_t)path + offset));
        }
    }

    if (!result.found) {
        klog('w', "%s not found", path);klog_flush();
    }

    return result;
}

#define VFS_LOOKUP(lk, path, ret) \
    auto lk = lookup(path); \
    if (!lk.found) {        \
        seterr(ENOENT);     \
        return ret;         \
    }

#define VFS_LOOKUP_R(lk, path)  \
    VFS_LOOKUP(lk, path, NULL)

#define VFS_LOOKUP_V(lk, path)  \
    VFS_LOOKUP(lk, path, )


StreamFile* VFS::open(char* path, int flags) {
    VFS_LOOKUP_R(lk, path);
    return lk.fs->open(lk.path, flags);
}

Directory* VFS::opendir(char* path) {
    VFS_LOOKUP_R(lk, path);
    return lk.fs->opendir(lk.path);
}


void VFS::rename(char* opath, char* npath) {
    VFS_LOOKUP_V(olk, opath);
    VFS_LOOKUP_V(nlk, npath);
    
    if (olk.fs != nlk.fs) {
        seterr(EXDEV);
        return;
    }

    olk.fs->rename(opath, npath);
}

void VFS::unlink(char* path) {
    VFS_LOOKUP_V(lk, path);
    lk.fs->unlink(lk.path);
}
