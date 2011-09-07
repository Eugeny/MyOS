#include "kutils.h"
#include "gdt.h"
#include "idt.h"

#include "terminal.h"


//    void print(Terminal *t, char*s) { t->write(s); t->draw(); }
    void print(Terminal *t, char*s) { kprints(s); }
    Terminal t;
    
extern "C" void kmain (void* mbd, unsigned int magic)
{
    if (magic != 0x2BADB002)
    {
    }
    
    t.reset();

    print(&t, "Setting GDT\n");
    gdt_init();

    print(&t, "Setting IDT\n");
    idt_init();
        
    for(;;);
    print(&t, "In protected mode\n");
    
    char s[] = "Test  \n\0";
    int c = 0;
    while (1) {
        s[4] = (char)((int)'0' + c++%10);
        t.write(s);
        t.draw();
        kprints(s);
//        for (int i=0;i<10000000;i++);
    }
///    putchar((int)"o");
    for(;;);
}
