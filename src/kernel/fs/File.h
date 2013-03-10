#ifndef FS_FILE_H
#define FS_FILE_H

#include <lang/lang.h>
#include <sys/stat.h>


class FS;

#define FILE_STREAM 0
#define FILE_DIRECTORY 1


class File {
public:
    File(const char* path, FS*);
    virtual void close();
    virtual int stat(struct stat* stat);

    int type;
    int refcount;
private:
    char path[1024];
    FS* filesystem;
};


class StreamFile : public File {
public:
    StreamFile(const char*, FS*);
    virtual void write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual bool canRead();
};


class StaticFile : public StreamFile {
public:
    StaticFile(void*, uint64_t);
    virtual void write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count) ;
    virtual void close();
    virtual bool canRead();
    virtual int stat(struct stat* stat);
private:
    void* content;
    uint64_t offset, size;
};

#endif