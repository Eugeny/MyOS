#include <vfs/VFS.h>
#include <kutils.h>


void VFS::init() {
    mounts = new LinkedList<Mountpoint*>();
}

LinkedList<char*>* VFS::splitPath(char* p) {
    LinkedList<char*>* r = new LinkedList<char*>();

    char tmp[256] = "/";
    char* pp = tmp;
    while (*p++) {
        if (*p == '/') {
            *pp++ = 0;
            r->insertLast(strdup(tmp));
            pp = tmp;
        } else {
            *pp++ = *p;
        }
    }

    *pp++ = 0;

    if (strlen(tmp) > 0)
        r->insertLast(strdup(tmp));

    return r;
}

LinkedList<char*>* VFS::listFiles(char* path) {
    Mountpoint* m = getFS(path, true);
    char* p = path + strlen(m->path);
    return m->fs->listFiles(p);
}

Stat* VFS::stat(char* path) {
    Mountpoint* m = getFS(path, false);
    char* p = path + strlen(m->path);
    return m->fs->stat(p);
}

FileObject* VFS::open(char* path, int mode) {
    Mountpoint* m = getFS(path, false);
    char* p = path + strlen(m->path);
    return m->fs->open(p, mode);
}

Mountpoint* VFS::getFS(char* p, bool mounted) {
    if (strcmp(p, (char*)"/"))
        return mounts->get(0);

    int best = 0;
    Mountpoint* res = NULL;
    LinkedListIter<Mountpoint*>* i = mounts->iter();
    for (; !i->end(); i->next()) {
        Mountpoint* m = i->get();
        if (strlen(m->path) >= best && strstarts(p, m->path) && (mounted || !strcmp(p, m->path))) {
            best = strlen(m->path);
            res = m;
        }
    }

    delete i;
    return res;
}

void VFS::mount(FS* fs, char* path) {
    Mountpoint* m = new Mountpoint();
    m->fs = fs;
    m->path = path;
    mounts->insertLast(m);
}
