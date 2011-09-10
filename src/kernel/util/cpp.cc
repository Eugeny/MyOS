#include <memory/Heap.h>
#include <kutils.h>

void *__dso_handle;

// Defined in the linker.
extern "C" u32int start_ctors;
extern "C" u32int end_ctors;

void initialiseConstructors()
{
    u32int *iterator = reinterpret_cast<u32int*>(&start_ctors);
    while (iterator < reinterpret_cast<u32int*>(&end_ctors))
        (reinterpret_cast<void (*)(void)>(*iterator++))();
}

extern "C" void __cxa_atexit(void (*f)(void *), void *p, void *d) { }
extern "C" void __cxa_pure_virtual() {}
extern "C" int __cxa_guard_acquire() { return 1; }
extern "C" void __cxa_guard_release() { }

void *operator new (size_t size) {
    return kmalloc(size);
}

void *operator new[] (size_t size) throw() {
    return kmalloc(size);
}

void *operator new (size_t size, void* memory) {
    return memory;
}

void *operator new[] (size_t size, void* memory) {
    return memory;
}

void operator delete (void * p) {
    kfree(p);
}

void operator delete[] (void * p) {
    kfree(p);
}

void operator delete (void *p, void *q) {
    PANIC("Operator delete -implement");
}

void operator delete[] (void *p, void *q) {
    PANIC("Operator delete[] -implement");
}
