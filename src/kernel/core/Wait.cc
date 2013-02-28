#include <core/Wait.h>
#include <hardware/pit/PIT.h>


bool WaitForever::isComplete() {
    return false;
}

WaitForDelay::WaitForDelay(uint64_t d) {
    delay = d;
    started = PIT::get()->getTime();
}

bool WaitForDelay::isComplete() {
    return PIT::get()->getTime() - started > delay;
}