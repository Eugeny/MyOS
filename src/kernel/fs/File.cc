#include <fs/File.h>
#include <string.h>

void File::close() {
    
}


StaticFile::StaticFile(void* c, uint64_t s) {
    content = c;
    size = s;
    offset = 0;
}

void StaticFile::write(const void* buffer, uint64_t count) {
}

uint64_t StaticFile::read(void* buffer, uint64_t count) {
    int c = 0;
    if (size - offset < count)
        c = size - offset;
    memcpy(buffer, (void*)((uint64_t)content + offset), c);
    offset += c;
    return c;
}

void StaticFile::close() {
}

bool StaticFile::canRead() {
    return true;
}