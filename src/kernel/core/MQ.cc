#include <core/MQ.h>
#include <kutil.h>

Message::Message(char* s) {
    id = s;
}

void Message::post(void* d) {
    for (MessageConsumer c : consumers) {
    //for (auto i = begin(consumers); i != end(consumers); ++i) {
      //  MessageConsumer c = *i; 
        c(d);
    }
}

void Message::registerConsumer(MessageConsumer h) {
    consumers.add(h);
}
 