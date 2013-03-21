#ifndef FS_DEVFS_DEVFS_H
#define FS_DEVFS_DEVFS_H

#include <fs/FS.h>
#include <fs/File.h>
#include <fs/Directory.h>
#include <fs/devfs/PTY.h>


class DevFS : public FS {
public:
    DevFS();
    virtual char* getName();
    virtual StreamFile* open(char* path, int flags);
    virtual Directory* opendir(char* path);
};  


class NullSource : public StreamFile {
public:
    NullSource(const char* path, FS*);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual void close();
    virtual bool canRead();
};


#endif