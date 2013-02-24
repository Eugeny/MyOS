global loader, end
extern _start, _end

section .text

[BITS 32]


ALIGN 16
MbHdr:
    ; Magic
    DD  0xE85250D6
    ; Architecture
    DD  0
    ; Length
    DD  HdrEnd - MbHdr
    ; Checksum
    DD  -(0xE85250D6 + 0 + (HdrEnd - MbHdr))
 
    ;
    ; Tags
    ;
    _mbir_start:
    DW  1, 0
    DD  16
    DD  3,1
    _mbir_end:

    ; End Of Tags
    DW  0, 0
    DD  8
 
    ; Hdr End Mark
HdrEnd:




; CODE --------------------------------
; 32 bit bootstrap

loader:
    ;cli
    ;push    esp
    ;push    ebx

    mov dword [0xb8000], 0x07690748

    push ebx
    call loader
    
    jmp $


section .bss
align 4

end: