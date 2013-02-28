#ifndef CORE_WAIT_H
#define CORE_WAIT_H

#include <lang/lang.h>


class Wait {
public:
    virtual bool isComplete() = 0;
};

class WaitForever : public Wait {
public:
    virtual bool isComplete();
};

class WaitForDelay : public Wait {
public:
    WaitForDelay(uint64_t ms);
    virtual bool isComplete();
private:
    uint64_t started, delay;
};

#endif