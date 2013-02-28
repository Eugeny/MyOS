#include <core/MQ.h>
#include <kutil.h>

std::map<const char*, std::vector<MessageConsumer>*>* MQ::consumers = 0;


void MQ::post(const char* id, void* arg) {
    if (!MQ::hasMessage(id)) {
        klog('e', "Message %s is not registered!", id);
        return;
    }
    for (MessageConsumer &c : *((*consumers)[id])) {
        //klog('i',"Sending %s to %lx", id,c);
        c(arg);
    }
}


void MQ::registerMessage(const char* id) {
    if (!consumers)
        consumers = new std::map<const char*, std::vector<MessageConsumer>*>();
    //klog('t',"Registering %s %lx",id,(*consumers)[id]);
    (*consumers)[id] = new std::vector<MessageConsumer>();
}

bool MQ::hasMessage(const char* id) {
    return consumers->count(id) != 0;
}

void MQ::registerConsumer(const char* id, MessageConsumer c) {
    if (!MQ::hasMessage(id)) {
        klog('e', "Message %s is not registered!", id);
        return;
    }
    //klog('t',"Registering %lx for %s", c,id);
    (*consumers)[id]->push_back(c);
}
