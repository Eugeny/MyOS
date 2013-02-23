#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <string>


int main() {}


extern "C" void kmain (void* mbd, int sp) {
    KTRACEMSG("started")
    KTRACEMSG("before vector init")
    
    KTRACEMEM
    auto i = new std::vector<std::string> { "one", "two", "three" };
    KTRACEMEM

    KTRACEMSG("after vector init")

    for (std::string &s : *i) {
        KTRACEMSG(s.c_str());
    }

    KTRACEMEM
    delete i;
    KTRACEMEM
}