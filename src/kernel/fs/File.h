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
    virtual bool isEOF();
    virtual void fdClosed();

    int type;
    int refcount;
    int flags;
protected:
    char path[1024];
    FS* filesystem;
};


class StreamFile : public File {
public:
    StreamFile(const char*, FS*);
    virtual int write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual uint64_t seek(uint64_t offset, uint64_t whence);
    virtual bool canRead();
};


class StaticFile : public StreamFile {
public:
    StaticFile(void*, uint64_t);
    virtual uint64_t read(void* buffer, uint64_t count) ;
    virtual void close();
    virtual bool canRead();
    virtual int stat(struct stat* stat);
private:
    void* content;
    uint64_t offset, size;
};

#endif