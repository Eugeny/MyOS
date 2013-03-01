#include <fs/Pipe.h>
#include <kutil.h>
#include <string.h>


Pipe::Pipe() {
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
    return bufferLength > 0;
}
