#ifndef MEMORY_GDT_H
#define MEMORY_GDT_H

#include <lang/lang.h>
#include <lang/Singleton.h>


class GDT : public Singleton<GDT> {
public:
    void init();
    void setDefaults();
    void setGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
    void flush();
};

#endif
