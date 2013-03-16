#ifndef FS_DEVFS_RANDOMSOURCE_H
#define FS_DEVFS_RANDOMSOURCE_H

#include <fs/File.h>
#include <fs/FS.h>


class RandomSource : public StreamFile {
public:
    RandomSource(const char* path, FS*);
    virtual uint64_t read(void* buffer, uint64_t count) ;
    virtual void close();
    virtual bool canRead();
};

#endif