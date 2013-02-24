#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <map>
#include <string>

#include <tty/Terminal.h>
#include <tty/Escape.h>
#include <tty/PhysicalTerminalManager.h>


int main() {}


extern "C" void kmain (void* mbd, int sp) {
    //auto i = new std::vector<std::string> { "one", "two" };

    //for (std::string &s : *i) {
        //KTRACEMSG(s.c_str());
    //}

    PhysicalTerminalManager::get()->init(5);
    klog_init();

    KTRACEMEM
    //auto i = new std::map<std::string,std::string> { { "a", "b" }, { "c", "d"} };
    //auto i = new std::map<char*,char*> { { "a", "b" }, { "c", "d"} };
    //KTRACEMEM
    //delete i;
    //KTRACEMEM

    Terminal* t = PhysicalTerminalManager::get()->getActiveTerminal();

    t->write(Escape::C_B_RED);
    t->write(":K: ");
    t->write(Escape::C_B_GREEN);
    t->write(":G: ");
    t->write(Escape::C_B_BLUE);
    t->write(":B: ");
    t->write(Escape::C_B_YELLOW);
    t->write(":Y: ");
    t->write(Escape::C_OFF);
    t->write("test\n");
    t->render();
}