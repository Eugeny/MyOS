#ifndef CORE_MQ_H
#define CORE_MQ_H

#include <lang/lang.h>
#include <lang/Pool.h>


typedef void(*MessageConsumer)(void*);


class Message {
public:
    Message(char*);
    void post(void*);
    void registerConsumer(MessageConsumer);
private:
    char* id;
    Pool<MessageConsumer, 16> consumers; 
};


#endif