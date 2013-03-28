#include <fs/devfs/DevFS.h>
#include <fs/devfs/RandomSource.h>
#include <fs/File.h>
#include <core/Process.h>
#include <core/Scheduler.h>
#include <string.h>
#include <kutil.h>
#include <errno.h>


DevFS::DevFS() {
}

char* DevFS::getName() {
    return "DevFS";
}

StreamFile* DevFS::open(char* path, int flags) {
    if (strcmp(path, "/tty") == 0 || strcmp(path, "/console") == 0)
        return Scheduler::get()->getActiveThread()->process->pty->openSlave();
    if (strcmp(path, "/null") == 0)
        return new NullSource("/null", this);
    if (strcmp(path, "/random") == 0)
        return new RandomSource("/random", this);
    if (strcmp(path, "/urandom") == 0)
        return new RandomSource("/urandom", this);
    klog('w', "DevFS entry not found: %s", path);
    seterr(ENOENT);
    return NULL;
}

Directory* DevFS::opendir(char* path) {
    seterr(ENOENT);
    return NULL;
}

NullSource::NullSource(const char* path, FS* fs) : StreamFile(path, fs) {}

uint64_t NullSource::read(void* buffer, uint64_t count) {
    return 0;
}

void NullSource::close() {
}

bool NullSource::canRead() {
    return true;
}