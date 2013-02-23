#ifndef HARDWARE_KEYBOARD_H
#define HARDWARE_KEYBOARD_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <interrupts/Interrupts.h>

#define KBD_MOD_CTRL 1
#define KBD_MOD_SHIFT 2
#define KBD_MOD_ALT 4
#define KBD_MOD_META 8

typedef void(*keyboard_handler)(u32int, u32int);

class Keyboard : public Singleton<Keyboard> {
public:
    void init();
    void handle(isrq_registers_t*);
    void setHandler(keyboard_handler h);
private:
    bool _escaping;
    u32int _modifiers;
    u32int _buffer;
    keyboard_handler _handler;
};

#endif
