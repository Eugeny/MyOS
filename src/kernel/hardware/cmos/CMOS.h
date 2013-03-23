#ifndef HARDWARE_CMOS_CMOS_H
#define HARDWARE_CMOS_CMOS_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <time.h>


class CMOS : public Singleton<CMOS> {
public:
    uint8_t readRegister(uint8_t index);
    time_t readTime();
};

#endif
