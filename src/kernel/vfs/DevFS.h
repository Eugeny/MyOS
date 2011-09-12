#ifndef VFS_DEVFS_H
#define VFS_DEVFS_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <util/Singleton.h>
#include <io/FileObject.h>
#include <tty/TTY.h>
#include <vfs/FS.h>
#include <vfs/Stat.h>


class DeviceEntry {
public:
    FileObject* file;
    char* name;
};


class DevFS : public FS {
public:
    DevFS();
    virtual LinkedList<char*>* listFiles(char* node);
    virtual Stat* stat(char* path);
    virtual FileObject* open(char* path, int mode);
    void registerDevice(FileObject* f, char* name);
private:
    LinkedList<DeviceEntry*>* devices;
};


class DevFSMaster : public Singleton<DevFSMaster> {
public:
    void init();
    FS* getFS();
    void addTTY(TTY* t);
private:
    DevFS* fs;
};


#endif
