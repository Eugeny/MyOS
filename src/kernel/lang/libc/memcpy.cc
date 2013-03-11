#include <lang/lang.h>

extern "C" void* __wrap_memcpy(void* dest, const void* src, uint64_t count) {
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;
    --src8;
    --dst8;
    
    while (count--) {
        *++dst8 = *++src8;
    }
    return dest;
}