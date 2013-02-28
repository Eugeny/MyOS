#ifndef HARDWARE_KEYBOARD_H
#define HARDWARE_KEYBOARD_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <interrupts/Interrupts.h>

#define KBD_MOD_CTRL 1
#define KBD_MOD_SHIFT 2
#define KBD_MOD_ALT 4
#define KBD_MOD_META 8



struct keyboard_event_t {
    uint64_t mods, scancode;
};


class Keyboard : public Singleton<Keyboard> {
public:
    void init();
    void handle(isrq_registers_t*);
    static const char* MSG_KEYBOARD_EVENT;
private:
    bool _escaping;
    uint64_t _modifiers;
    uint64_t _buffer;
};

#endif
