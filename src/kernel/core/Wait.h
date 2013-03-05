#ifndef CORE_WAIT_H
#define CORE_WAIT_H

#include <lang/lang.h>
#include <fs/File.h>


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


class WaitForFile : public Wait {
public:
    WaitForFile(File* f);
    virtual bool isComplete();
private:
    File* file;
};
#endif