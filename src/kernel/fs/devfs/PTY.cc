#include <fs/devfs/PTY.h>
#include <fs/FS.h>
#include <lang/lang.h>


PTY::PTY() {

}

PTYMaster* PTY::openMaster() {
    return new PTYMaster(this);
}

PTYSlave* PTY::openSlave() {
    return new PTYSlave(this);
}


PTYMaster::PTYMaster(PTY* p) : StreamFile(0, 0) {
    pty = p;
}

int PTYMaster::write(const void* buffer, uint64_t count) {
    return pty->masterWritePipe.write(buffer, count);
}

uint64_t PTYMaster::read(void* buffer, uint64_t count) {
    return pty->masterReadPipe.read(buffer, count);
}

bool PTYMaster::canRead() {
    return pty->masterReadPipe.canRead();
}



PTYSlave::PTYSlave(PTY* p) : StreamFile(0, 0) {
    pty = p;
}

int PTYSlave::write(const void* buffer, uint64_t count) {
    return pty->masterReadPipe.write(buffer, count);
}

uint64_t PTYSlave::read(void* buffer, uint64_t count) {
    return pty->masterWritePipe.read(buffer, count);
}

bool PTYSlave::canRead() {
    return pty->masterWritePipe.canRead();
}

int PTYSlave::stat(struct stat* stat) {
    File::stat(stat);
    stat->st_mode |= S_IFCHR;
    return 0;
}
