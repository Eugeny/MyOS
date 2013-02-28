#include <hardware/keyboard/Keyboard.h>
#include <core/MQ.h>
#include <hardware/io.h>
#include <interrupts/Interrupts.h>
#include <kutil.h>


const char* Keyboard::MSG_KEYBOARD_EVENT = "keyboard-event";


void Keyboard::handle(isrq_registers_t *r) {
    uint8_t scancode = inb(0x60);

    if (_escaping)
        _buffer *= 256;
    else
        _buffer = 0;
    _escaping = false;
    _buffer += scancode;

    if (_buffer == 0xE0) _escaping = true;

    else if (_buffer == 0x1D) _modifiers |=  KBD_MOD_CTRL;
    else if (_buffer == 0x9D) _modifiers &= !KBD_MOD_CTRL;
    else if (_buffer == 0x2A) _modifiers |=  KBD_MOD_SHIFT;
    else if (_buffer == 0xAA) _modifiers &= !KBD_MOD_SHIFT;
    else if (_buffer == 0x38) _modifiers |=  KBD_MOD_ALT;
    else if (_buffer == 0xB8) _modifiers &= !KBD_MOD_ALT;
    else if (_buffer == 0xE05B) _modifiers |=  KBD_MOD_META;
    else if (_buffer == 0xE0DB) _modifiers &= !KBD_MOD_META;

    else if (scancode >= 0x80) {
        static keyboard_event_t event;
        event.mods = _modifiers;
        event.scancode = scancode;
        MQ::post(MSG_KEYBOARD_EVENT, (void*)&event);
    }
}

static void kbdh(isrq_registers_t *r) {
    Keyboard::get()->handle(r);
}

void Keyboard::init() {
    MQ::registerMessage(MSG_KEYBOARD_EVENT);
    Interrupts::get()->setHandler(IRQ(1), kbdh);
    _buffer = 0;
    _modifiers = 0;
    _escaping = false;
}
