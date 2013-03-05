#include <fs/vfs/VFS.h>
#include <string.h>
#include <kutil.h>
#include <errno.h>


void VFS::mount(char* point, FS* fs) {
    auto m = new mount_t();
    strcpy(m->path, point);
    m->fs = fs;
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

File* VFS::open(char* path, int flags) {
    auto lk = lookup(path);
    if (!lk.found) {
        seterr(ENOENT);
        return NULL;
    }
    return lk.fs->open(lk.path, flags);
}

Directory* VFS::opendir(char* path) {
    auto lk = lookup(path);
    if (!lk.found) {
        seterr(ENOENT);
        return NULL;
    }
    return lk.fs->opendir(lk.path);
}
