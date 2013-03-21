#include <fs/FS.h>
#include <kutil.h>
#include <errno.h>


char* FS::getName() {
    return "Unknown FS";
}

void FS::rename(char* opath, char* npath) {
    seterr(EROFS);
}

void FS::unlink(char* path) {
    seterr(EROFS);
}
