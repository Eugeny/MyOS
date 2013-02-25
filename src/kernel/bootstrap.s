global loader, end
extern _start, _end, kmain


section .data
    stack resb 102400
    stacktop dd 0

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
    cli
    mov  eax, stacktop
    mov  esp, eax

    lgdt    [cs:GDTR]  
         mov     eax,cr0                 ; switch to protected mode 
        or      al,1 
        mov     cr0,eax 
    jmp     CODE_SELECTOR:gdt_flushed

gdt_flushed:
    mov     eax,DATA_SELECTOR       ; load 4 GB data descriptor 
    mov     ds,ax                   ; to all data segment registers 
    mov     es,ax 
    mov     fs,ax 
    mov     gs,ax 
    mov     ss,ax 


    ; Zero out pages
    mov     edi, 1000h 
    mov     ecx, 80000h 
    xor     eax, eax 
    rep     stosd                   

    mov     dword [1000h], 2000h + 111b ; first PDP table 
    mov     dword [2000h], 3000h + 111b ; first page directory 

    mov     edi, 3000h
    mov     edx, 4000h + 111b
    mov     ecx, 20

_loop_make_pd_entries:
    mov     dword [edi], edx
    add     edx, 1000h
    add     edi, 8h
    dec     ecx
    cmp     ecx, 0
    jnz     _loop_make_pd_entries

    mov     edi,4000h              ; address of first page table 
    mov     eax,0 + 111b 
    mov     ecx,512*20                ; number of pages to map (1 MB) 
_loop_make_page_entries: 
    stosd 
    add     edi,4 
    add     eax,1000h 
    loop    _loop_make_page_entries 



    ; Load PML4
    mov     eax, 1000h 
    mov     cr3, eax

    ; Enable Long mode
    mov     ecx,0C0000080h          ; EFER MSR 
    rdmsr 
    ;or      eax, 1 >> 8             ; enable long mode 
    bts eax,8
    wrmsr 

    ; Enable PAE
    mov     eax, cr4 
    or      eax, 32
    mov     cr4, eax

    ; Enable paging
    mov     eax,cr0 
    ;or      eax, 1 << 31
    bts     eax, 31
    mov     cr0,eax                 ; enable paging 

    jmp     LONG_SELECTOR:loader64 


bits 64

loader64:

        mov     rax,'L O N G ' 
        mov     [0B8000h],rax 

    call    kmain
    jmp     $


NULL_SELECTOR equ 0 
DATA_SELECTOR equ 1 << 3                 ; flat data selector (ring 0) 
CODE_SELECTOR equ 2 << 3                 ; 32-bit code selector (ring 0) 
LONG_SELECTOR equ 3 << 3  
GDTR:                                   ; Global Descriptors Table Register 
    dw 4*8-1                              ; limit of GDT (size minus one) 
    dq GDT                                ; linear address of GDT 

GDT:
    dw 0,0,0,0                                ; null desciptor 
    dw 0FFFFh,0,9200h,08Fh              ; flat data desciptor 
    dw 0FFFFh,0,9A00h,0CFh              ; 32-bit code desciptor 
    dw 0FFFFh,0,9A00h,0AFh              ; 64-bit code desciptor 




section .bss
align 4

end: