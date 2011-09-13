#include "Lock.h"

Lock::Lock() {
    locked = false;
}

void Lock::acquire() {
    while (locked) ;
    locked = true;
}

bool Lock::attempt() {
    if (!locked) {
        acquire();
        return true;
    }
    return false;
}

void Lock::release() {
    locked = false;
}
