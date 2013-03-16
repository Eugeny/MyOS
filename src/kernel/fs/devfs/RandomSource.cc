#include <fs/devfs/RandomSource.h>
#include <stdlib.h>

RandomSource::RandomSource(const char* path, FS* fs) : StreamFile(path, fs) {}

uint64_t RandomSource::read(void* buffer, uint64_t count) {
    for (uint64_t i = 0; i < count; i++)
        ((uint8_t*)buffer)[i] = random() % 256;
    return count;
}

void RandomSource::close() {
}

bool RandomSource::canRead() {
    return true;
}