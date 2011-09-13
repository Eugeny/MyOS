#include <util/cpp.h>

class Lock {
public:
    Lock();
    void acquire();
    bool attempt();
    void release();
private:
    bool locked;
};
