#define uint32_t unsigned int


extern "C" void boot(uint32_t *grub_start) {
    char *vram = (char *)0xb8000;
    
    uint32_t *max = grub_start + (uint32_t)*grub_start;
    grub_start += 8;
    //while (*grub_start != 0 ) {
    while (grub_start <max ) {
        
        if (*grub_start == 3) {
            grub_start += 16;
            uint32_t address = *(grub_start + 8);
            *vram = '+';
        vram +=2;
            //return;
        } else {
            for(int i=0;i< *grub_start;i++){

        *vram = '.';
        vram +=2;
    }
            grub_start += (*(grub_start + 4) + 7) & ~7;
*vram = '-';
        vram +=2;
        }
    }

*vram = 'x';
        vram +=2;


}