#ifndef CORE_MQ_H
#define CORE_MQ_H

#include <lang/lang.h>

#include <map>
#include <vector>


class Message {
public:
    Message(char*, void*);
    char* id;
    void* arg;
};


typedef void(*MessageConsumer)(void*);


class MQ {
public:
    static void post(const char*, void*);
    static void registerMessage(const char*);
    static void registerConsumer(const char*, MessageConsumer);
    static bool hasMessage(const char* id);
private:
    static std::map<const char*, std::vector<MessageConsumer>*>* consumers;
};

#endif