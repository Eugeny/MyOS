#include <lang/lang.h>

extern "C" void _Unwind_Resume(struct _Unwind_Exception *object){};
extern "C" int _Unwind_Backtrace (void*, void*) { return 0; }
extern "C" void* _Unwind_GetIP (void*) { return 0; }
extern "C" uint16_t _Unwind_GetCFA (void *) { return 0; }
extern "C" int __gcc_personality_v0(int version, int actions,
         uint64_t exceptionClass, void* exceptionObject,
         void* context) {return 0;}

extern "C" void __cxa_pure_virtual() { while (1); }
extern "C" void __stack_chk_fail(void) {}
extern "C" void __fdelt_chk(void) {}

void *__dso_handle;

namespace __cxxabiv1 
{
    /* guard variables */
 
    /* The ABI requires a 64-bit type.  */
    __extension__ typedef int __guard __attribute__((mode(__DI__)));
 
    extern "C" int __cxa_guard_acquire (__guard *);
    extern "C" void __cxa_guard_release (__guard *);
    extern "C" void __cxa_guard_abort (__guard *);
 
    extern "C" int __cxa_guard_acquire (__guard *g) 
    {
        return !*(char *)(g);
    }
 
    extern "C" void __cxa_guard_release (__guard *g)
    {
        *(char *)g = 1;
    }
 
    extern "C" void __cxa_guard_abort (__guard *)
    {
 
    }
}