#include <fs/FS.h>
#include <kutil.h>
#include <errno.h>


void FS::unlink(char* path) {
    seterr(EROFS);
}
