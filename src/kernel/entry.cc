#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <string>


int main() {}


extern "C" void kmain (void* mbd, int sp) {
    KTRACEMSG("started");

    char buf[1024];
    sprintf(buf, "test");
    sout(buf);

    sout("before vector init");
    
    std::vector<std::string> i { "one", "two", "three" };
    sout("after vector init");

    for (std::string &s : i) {
        sout(s.c_str());
    }
}