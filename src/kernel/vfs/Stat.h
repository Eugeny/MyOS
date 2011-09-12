#ifndef VFS_STAT_H
#define VFS_STAT_H

#include <util/cpp.h>
#include <util/LinkedList.h>


class Stat {
public:
    Stat() {
        isDirectory = false;
        isTTY = false;
        isDevice = false;
        size = 0;
    }

    bool isDirectory;
    bool isTTY;
    bool isDevice;
    u32int size;
};


#endif
