#ifndef CORE_WAIT_H
#define CORE_WAIT_H

#include <util/cpp.h>

class Wait {
public:
	virtual bool  check() { return false; }
	virtual char *toString() { return NULL; }
};

class WaitForever : public Wait {
public:
	virtual bool  check() { return false; }
	virtual char *toString() { return strdup("forever"); }
};

#endif