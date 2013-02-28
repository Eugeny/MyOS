#ifndef FS_FAT32_FAT32FS_H
#define FS_FAT32_FAT32FS_H

#include <fs/FS.h>
#include <fs/File.h>
#include <fs/Directory.h>
#include <libfat/ff.h>


class FAT32FS : public FS {
public:
    FAT32FS();
    virtual File* open(char* path, int flags);
    virtual Directory* opendir(char* path);
private:
    FATFS* fs;
};  


class FAT32File : public File {
public:
    FAT32File(FAT32FS* fs, FIL* f);
    virtual void write(void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual void close();
private:
    FIL* fil;
};

class FAT32Directory : public Directory {
public:
    FAT32Directory(FAT32FS* fs, FDIR* d);

    virtual struct dirent* read();
    virtual void close();
private:
    FDIR* dir;
};



#endif