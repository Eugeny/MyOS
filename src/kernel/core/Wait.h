#ifndef CORE_WAIT_H
#define CORE_WAIT_H

#include <lang/lang.h>
#include <fs/File.h>


#define WAIT_FOREVER 0
#define WAIT_FOR_DELAY 1
#define WAIT_FOR_FILE 2
#define WAIT_FOR_CHILD 3


class Wait {
public:
    virtual bool isComplete() = 0;
    int type;
};


class WaitForever : public Wait {
public:
    WaitForever();
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
    WaitForFile(StreamFile* f);
    virtual bool isComplete();
private:
    StreamFile* file;
};


class WaitForChild : public Wait {
public:
    WaitForChild(uint64_t p);
    virtual bool isComplete();
private:
    uint64_t pid;
};
#endif