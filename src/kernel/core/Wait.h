#ifndef CORE_WAIT_H
#define CORE_WAIT_H

#include <util/cpp.h>
#include <kutils.h>

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

class WaitPID : public Wait {
public:
	WaitPID(int pid) {
		this->pid = pid;
	}

	virtual bool check();

	virtual char *toString() { 
		return strdup(to_dec(pid)); 
	}

private:
	int pid;
};

#endif