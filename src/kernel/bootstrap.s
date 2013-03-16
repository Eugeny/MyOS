global loader, end
extern _start, _end, kmain


section .data
    stack times 409600 db 0
    stacktop dd 0

    global multiboot_info
    multiboot_info dd 0


section .text

[BITS 32]

MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
VIDEO       equ  1<<2
FLAGS       equ  MODULEALIGN | MEMINFO | VIDEO
MAGIC       equ  0x1BADB002             ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required
 
section .text
 
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

    dd 0,0,0,0,0

    dd 1
    dd 40
    dd 25
    dd 0



; CODE --------------------------------
; 32 bit bootstrap

loader:
    cli
    mov     eax, stacktop
    mov     esp, eax

    lgdt    [cs:GDTR]  

    ; Ensure protected mode
    mov     eax, cr0
    or      al, 1 
    mov     cr0, eax 

    ; Flush GDT
    jmp     CODE_SELECTOR:gdt_flushed

gdt_flushed:
    mov     eax, DATA_SELECTOR       
    mov     ds, ax                   
    mov     es, ax 
    mov     fs, ax 
    mov     gs, ax 
    mov     ss, ax 

    ;mov     [es:multiboot_info], ebx

    ; Zero out temporary pages
    mov     edi, 1000h 
    mov     ecx, 80000h 
    xor     eax, eax 
    rep     stosd                   

    ; Identity map first megabytes
    mov     dword [1000h], 2000h + 111b ; first PDP table 
    mov     dword [2000h], 3000h + 111b ; first page directory 

    mov     edi, 3000h
    mov     edx, 4000h + 111b
    mov     ecx, 25

    ; Make PDs
_loop_make_pd_entries:
    mov     dword [edi], edx
    add     edx, 1000h
    add     edi, 8h
    dec     ecx
    cmp     ecx, 0
    jnz     _loop_make_pd_entries

    ; Make page entries
    mov     edi, 4000h             
    mov     eax, 0 + 111b 
    mov     ecx, 512*8/2 ; 8 mb just in case
_loop_make_page_entries: 
    stosd 
    add     edi,4 
    add     eax,1000h 
    loop    _loop_make_page_entries 


    ; Load PML4
    mov     eax, 1000h 
    mov     cr3, eax

    ; Enable Long mode
    mov     ecx, 0C0000080h          ; EFER MSR 
    rdmsr 
    bts     eax, 8 ; 64 bit 
    bts     eax, 0 ; SYSCALL
    wrmsr 

    ; Enable PAE
    mov     eax, cr4 
    bts     eax, 5 ; PAE
    ;bts     eax, 16 ; FSGSBASE
    mov     cr4, eax

    ; Enable paging
    mov     eax, cr0 
    bts     eax, 31
    mov     cr0, eax

    ; Go 64
    jmp     KCODE_SELECTOR:loader64 


bits 64

loader64:
    mov     rax, '.p.p.p.p' 
    mov     [0B8000h], rax 

    mov     rax, KDATA_SELECTOR
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    mov     rax, cr4 
    ;bts     rax, 20
    ;bts     rax, 16 // FSGSBASE not supported by virtualbox
    mov     cr4, rax

    mov     rdi, rbx ; multiboot_info
    call    kmain
    jmp     $


NULL_SELECTOR equ 0 
DATA_SELECTOR equ 1 << 3                 ; flat data selector (ring 0) 
CODE_SELECTOR equ 2 << 3                 ; 32-bit code selector (ring 0) 
KCODE_SELECTOR equ 3 << 3  
KDATA_SELECTOR equ 4 << 3  
UCODE_SELECTOR equ 5 << 3  
UDATA_SELECTOR equ 6 << 3  


GDTR:                                   ; Global Descriptors Table Register 
    dw 7*8-1                              ; limit of GDT (size minus one) 
    dq GDT                                ; linear address of GDT 

GDT:
    dw 0,0,0,0                             ; null desciptor 
    db 0xff, 0xff, 0, 0, 0, 0x92, 0x8f, 0  ; flat data 32
    db 0xff, 0xff, 0, 0, 0, 0x9a, 0xcf, 0  ; flat code 32
    db 0xff, 0xff, 0, 0, 0, 0x9a, 0xaf, 0  ; flat k code 64
    db 0xff, 0xff, 0, 0, 0, 0x92, 0xaf, 0  ; flat k data 64
    db 0xff, 0xff, 0, 0, 0, 0xfa, 0xaf, 0  ; flat u code 64
    db 0xff, 0xff, 0, 0, 0, 0xf2, 0xaf, 0  ; flat u data 64


section .bss
align 4

end: