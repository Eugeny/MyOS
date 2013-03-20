#ifndef FS_FAT32_FAT32FS_H
#define FS_FAT32_FAT32FS_H

#include <fs/FS.h>
#include <fs/File.h>
#include <fs/Directory.h>
#include <libfat/ff.h>


class FAT32FS : public FS {
public:
    FAT32FS();
    virtual StreamFile* open(char* path, int flags);
    virtual Directory* opendir(char* path);
    virtual void rename(char* opath, char* npath);
    virtual void unlink(char* path);
private:
    FATFS* fs;
};  


class FAT32File : public StreamFile {
public:
    FAT32File(const char*, FAT32FS*, FIL* f);
    ~FAT32File();
    virtual int write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual void close();
    virtual bool canRead();
    virtual int stat(struct stat* stat);
    virtual uint64_t seek(uint64_t offset, uint64_t whence);
    virtual bool isEOF();
private:
    bool eof;
    FIL* fil;
};

class FAT32Directory : public Directory {
public:
    FAT32Directory(const char* path, FAT32FS* f, FDIR*);
    virtual struct dirent* read();
    virtual void close();
    virtual int stat(struct stat* stat);
private:
    FAT32FS* filesystem;
    FDIR* dir;
};



#endif