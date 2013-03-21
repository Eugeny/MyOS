#include <fs/devfs/DevFS.h>
#include <fs/devfs/RandomSource.h>
#include <fs/File.h>
#include <core/Process.h>
#include <core/Scheduler.h>
#include <string.h>
#include <kutil.h>


DevFS::DevFS() {
}

char* DevFS::getName() {
    return "DevFS";
}

StreamFile* DevFS::open(char* path, int flags) {
    if (strcmp(path, "/tty") == 0)
        return Scheduler::get()->getActiveThread()->process->pty->openSlave();
    if (strcmp(path, "/random") == 0)
        return new RandomSource("/random", this);
    if (strcmp(path, "/urandom") == 0)
        return new RandomSource("/urandom", this);
    klog('w', "DevFS entry not found: %s", path);
    return NULL;
}

Directory* DevFS::opendir(char* path) {
    return NULL;
}
