#ifndef FS_PIPE_H
#define FS_PIPE_H

#include <lang/lang.h>
#include <fs/File.h>


#define PIPE_BUFFER_SIZE 10240


class Pipe : public StreamFile {
public:
    Pipe();
    virtual void write(const void* buffer, uint64_t count);
    virtual uint64_t read(void* buffer, uint64_t count);
    virtual bool canRead();
    virtual int stat(struct stat* stat);
private:
    uint8_t pipeBuffer[PIPE_BUFFER_SIZE];
    uint64_t bufferLength;
};


#endif