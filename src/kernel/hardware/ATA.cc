#include <hardware/ATA.h>
#include <kutils.h>


void lba2chs (u32int lba, int *c, int *h, int *s) {
    *c = lba/(63*16);
    *h = (lba/63) % 16;
    *s = (lba % 63) + 1;
}

void ata_read(u32int lba, u8int* buf) {
    int c, h, s;
    lba2chs(lba, &c, &h, &s);

    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x20);

    bool ready = false;
    do {
    TRACE    char st = inb(0x1f7);
        ready = st != 8;
    } while (!ready);

    for (int i = 0; i < 256; i++) {
        u16int word = inw(0x1f0);
        buf[i*2] = (char)(word/256);
        buf[i*2+1] = (char)(word%256);
    }
}/*

;
;       Writing to the hard disk using the ports!     by qark
;       +---------------------------------------+
;
;  The only differences between reading and writing using the ports is
;  that 30h is sent to the command register, and instead of INSW you
;  OUTSW.
;


   mov     dx,1f6h         ;Drive and head port
   mov     al,0a0h         ;Drive 0, head 0
   out     dx,al

   mov     dx,1f2h         ;Sector count port
   mov     al,1            ;Write one sector
   out     dx,al

   mov     dx,1f3h         ;Sector number port
   mov     al,1           ;Wrote to sector one
   out     dx,al

   mov     dx,1f4h         ;Cylinder low port
   mov     al,0            ;Cylinder 0
   out     dx,al

   mov     dx,1f5h         ;Cylinder high port
   mov     al,0            ;The rest of the cylinder 0
   out     dx,al

   mov     dx,1f7h         ;Command port
   mov     al,30h          ;Write with retry.
   out     dx,al
oogle:
   in      al,dx
   test    al,8            ;Wait for sector buffer ready.
   jz      oogle

   mov     cx,512/2        ;One sector /2
   mov     si,offset buffer
   mov     dx,1f0h         ;Data port - data comes in and out of here.
   rep     outsw           ;Send it.
*/
