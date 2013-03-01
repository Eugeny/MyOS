#include <fs/devfs/PTY.h>

PTY::PTY() {

}

PTYMaster* PTY::openMaster() {
    return new PTYMaster(this);
}

PTYSlave* PTY::openSlave() {
    return new PTYSlave(this);
}


PTYMaster::PTYMaster(PTY* p) {
    pty = p;
}

void PTYMaster::write(const void* buffer, uint64_t count) {
    pty->masterWritePipe.write(buffer, count);
}

uint64_t PTYMaster::read(void* buffer, uint64_t count) {
    return pty->masterReadPipe.read(buffer, count);
}

bool PTYMaster::canRead() {
    return pty->masterReadPipe.canRead();
}



PTYSlave::PTYSlave(PTY* p) {
    pty = p;
}

void PTYSlave::write(const void* buffer, uint64_t count) {
    pty->masterReadPipe.write(buffer, count);
}

uint64_t PTYSlave::read(void* buffer, uint64_t count) {
    return pty->masterWritePipe.read(buffer, count);
}

bool PTYSlave::canRead() {
    return pty->masterWritePipe.canRead();
}