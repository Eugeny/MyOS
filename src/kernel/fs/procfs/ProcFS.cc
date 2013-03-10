#include <core/Scheduler.h>
#include <core/Process.h>
#include <fs/procfs/ProcFS.h>
#include <fs/vfs/VFS.h>
#include <fs/File.h>
#include <string.h>
#include <kutil.h>


static char* CONTENT_OSRELEASE = "1.0\n";

ProcFS::ProcFS() {
}


StreamFile* ProcFS::open(char* path, int flags) {
    if (strcmp(path, "/sys/kernel/osrelease") == 0) {
        return new StaticFile(CONTENT_OSRELEASE, strlen(CONTENT_OSRELEASE));
    }
    if (strcmp(path, "/self/exe") == 0) {
        return VFS::get()->open(Scheduler::get()->getActiveThread()->process->exeName, flags);
    }
    klog('w', "ProcFS entry not found: %s", path);
    return NULL;
}

Directory* ProcFS::opendir(char* path) {
    return NULL;
}
