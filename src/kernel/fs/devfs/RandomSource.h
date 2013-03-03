#ifndef FS_DEVFS_RANDOMSOURCE_H
#define FS_DEVFS_RANDOMSOURCE_H

#include <fs/File.h>

class RandomSource : public File {
public:
    virtual void write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count) ;
    virtual void close();
    virtual bool canRead();
};

#endif