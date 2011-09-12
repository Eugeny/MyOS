#include <core/Thread.h>

extern "C" int fork();
extern "C" int newThread(thread_entry_point e, void* a);
