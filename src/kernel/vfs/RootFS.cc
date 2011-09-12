#include <vfs/RootFS.h>
#include <kutils.h>


RootFS::RootFS() {
}

LinkedList<char*>* RootFS::listFiles(char* node) {
    LinkedList<char*>* r = new LinkedList<char*>();
    if (strcmp(node, "/")) {
        r->insertLast("dev");
    }
    return r;
}

Stat* RootFS::stat(char* path) {
    Stat* s = new Stat();
    s->isDirectory = true;
    return s;
}
