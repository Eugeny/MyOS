#ifndef KUTILS_C 
#define KUTILS_C

void kprints(char* s) {
    unsigned char *vram = (unsigned char *)0xb8000;
    for (;*s;s++,vram+=2) {
        vram[0] = *s;
        vram[1] = 0xf;
    }
}

#endif
