global _ata_read
_ata_read:
   mov     dx,1f6h         ;Drive and head port
   mov     al,0a0h         ;Drive 0, head 0
   add     al, [esp+4]
   out     dx,al

   mov     dx,1f2h         ;Sector count port
   mov     al,1            ;Read one sector
   out     dx,al

   mov     dx,1f3h         ;Sector number port
   mov     al,[esp+5]            ;Read sector one
   out     dx,al

   mov     dx,1f4h         ;Cylinder low port
   mov     al,[esp+6]            ;Cylinder 0
   out     dx,al

   mov     dx,1f5h         ;Cylinder high port
   mov     al,[esp+7]            ;The rest of the cylinder 0
   out     dx,al

   mov     dx,1f7h         ;Command port
   mov     al,20h          ;Read with retry.
   out     dx,al
still_going:
   in      al,dx
   test    al,8            ;This means the sector buffer requires
            ;servicing.
   jz      still_going     ;Don't continue until the sector buffer
            ;is ready.

   mov     ecx,256        ;One sector /2
   mov     edi,[esp+8]
   mov     dx,1f0h         ;Data port - data comes in and out of here.
   rep     insw
   add esp, 8
   ret
