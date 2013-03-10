#ifndef FS_DIRECTORY_H
#define FS_DIRECTORY_H

#include <lang/lang.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fs/File.h>


class FS;

class Directory : public File {
public:
    Directory(const char*, FS*);
    virtual struct dirent* read() = 0;
    virtual void close() = 0;
    virtual int stat(struct stat* stat);
protected:
    struct dirent currentEntry;
};


#endif