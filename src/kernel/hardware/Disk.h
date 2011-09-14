#ifndef HARDWARE_DISK_H
#define HARDWARE_DISK_H

#include <util/cpp.h>
#include <util/Singleton.h>


class Disk : public Singleton<Disk> {
public:
    void init();
    void read(u32int lba, void* buf);
private:
    u32int base;
};

#endif
