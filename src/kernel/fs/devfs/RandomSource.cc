#include <fs/devfs/RandomSource.h>
#include <stdlib.h>


void RandomSource::write(const void* buffer, uint64_t count) {
}

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