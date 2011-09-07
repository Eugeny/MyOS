#include "kutils.c"
#include <stdio.h>
#include <string.h>


void kmain (void* mbd, unsigned int magic)
{
    if (magic != 0x2BADB002)
    {
        // Boot failed
    }
    
    kprints("Loaded!");
///    putchar((int)"o");
    for(;;);
}
