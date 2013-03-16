#include <fs/FS.h>
#include <kutil.h>
#include <errno.h>


void FS::rename(char* opath, char* npath) {
    seterr(EROFS);
}

void FS::unlink(char* path) {
    seterr(EROFS);
}
