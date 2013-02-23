#include <vfs/FS.h>

Stat* FS::stat(char* path) {
    return new Stat();
}
