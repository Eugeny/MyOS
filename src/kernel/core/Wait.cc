#include <core/Wait.h>
#include <hardware/pit/PIT.h>


WaitForever::WaitForever() {
    type = WAIT_FOREVER;
}

bool WaitForever::isComplete() {
    return false;
}



WaitForDelay::WaitForDelay(uint64_t d) {
    type = WAIT_FOR_DELAY;
    delay = d;
    started = PIT::get()->getTime();
}

bool WaitForDelay::isComplete() {
    return PIT::get()->getTime() - started > delay;
}



WaitForFile::WaitForFile(StreamFile* f) {
    type = WAIT_FOR_FILE;
    file = f;
}

bool WaitForFile::isComplete() {
    return file->canRead();
}



WaitForChild::WaitForChild(uint64_t p) {
    type = WAIT_FOR_CHILD;
    pid = p;
}

bool WaitForChild::isComplete() {
    return false;
}
