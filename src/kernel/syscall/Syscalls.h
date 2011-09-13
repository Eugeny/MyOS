#include <core/Thread.h>

extern "C" void kprint(char *);
extern "C" int fork();
extern "C" int newThread(thread_entry_point e, void* a);
