#include <vfs/DevFS.h>
#include <vfs/VFS.h>
#include <kutils.h>


DevFS::DevFS() {
    devices = new LinkedList<DeviceEntry*>();
}

LinkedList<char*>* DevFS::listFiles(char* node) {
    LinkedList<char*>* r = new LinkedList<char*>();

    if (strcmp(node, (char*)"")) {
        LinkedListIter<DeviceEntry*>* i = devices->iter();
        for (; !i->end(); i->next())
            r->insertLast(i->get()->name);
        delete i;
    }
    return r;
}

Stat* DevFS::stat(char* path) {
    path++;
    Stat* s = new Stat();
    s->isDevice = true;
    s->size = 0;
    LinkedList<char*>* p = VFS::splitPath(path);
    if (strstarts(p->get(p->length()-1), "tty"))
        s->isTTY = true;
    p->purge();
    delete p;

    return s;
}

void DevFS::registerDevice(FileObject* f, char* name) {
    DeviceEntry* e = new DeviceEntry();
    e->file = f;
    e->name = strdup(name);
    devices->insertLast(e);
}

FileObject* DevFS::open(char* path, int mode) {
    path++;
    LinkedListIter<DeviceEntry*>* i = devices->iter();
    for (; !i->end(); i->next())
        if (strcmp(i->get()->name, path)) {
            return i->get()->file;
            delete i;
        }
    delete i;
    return NULL;
}

void DevFSMaster::init() {
    fs = new DevFS();
}

FS* DevFSMaster::getFS() {
    return fs;
}

void DevFSMaster::addTTY(TTY *t) {
    static int index = 0;
    char name[5] = "ttyx";
    name[3] = '0' + (index++ % 10);
    fs->registerDevice(t, name);
}
