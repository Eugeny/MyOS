#include <fs/Pipe.h>
#include <kutil.h>
#include <string.h>


Pipe::Pipe() : File(0, 0) {
    bufferLength = 0;
}

void Pipe::write(const void* buffer, uint64_t count) {
    if (bufferLength + count > PIPE_BUFFER_SIZE) {
        klog('e', "Pipe overflow");
    }
    memcpy((void*)((uint64_t)pipeBuffer + bufferLength), buffer, count);
    bufferLength += count;
}

uint64_t Pipe::read(void* buffer, uint64_t count) {
    uint64_t c = (count < bufferLength) ? count : bufferLength;
    memcpy(buffer, pipeBuffer, c);
    memcpy(pipeBuffer, (void*)((uint64_t)pipeBuffer + c), PIPE_BUFFER_SIZE - c);
    bufferLength -= c;
    return c;
}

bool Pipe::canRead() {
    return true;//bufferLength > 0;
}


int Pipe::stat(struct stat* stat) {
    File::stat(stat);
    stat->st_mode |= S_IFIFO;
    return 0;
}
