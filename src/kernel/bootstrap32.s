
    lgdt    [cs:GDTR]  
    jmp     CODE_SELECTOR:gdt_flushed

gdt_flushed:
    mov     eax,DATA_SELECTOR       ; load 4 GB data descriptor 
    mov     ds,ax                   ; to all data segment registers 
    mov     es,ax 
    mov     fs,ax 
    mov     gs,ax 
    mov     ss,ax 

    ; Enable PAE
    mov     eax, cr4 
    or      eax, 32
    mov     cr4, eax

    ; Zero out pages
    mov     edi, 70000h 
    mov     ecx, 4000h >> 2 
    xor     eax, eax 
    rep     stosd                   

    mov     dword [70000h], 71000h + 111b ; first PDP table 
    mov     dword [71000h], 72000h + 111b ; first page directory 
    mov     dword [72000h], 73000h + 111b ; first page table 

    mov     edi,73000h              ; address of first page table 
    mov     eax,0 + 111b 
    mov     ecx,256*256                 ; number of pages to map (1 MB) 

_loop_make_page_entries: 
    stosd 
    add     edi,4 
    add     eax,1000h 
    loop    _loop_make_page_entries 

    ; Load PDPT
    mov     eax, 70000h 
    mov     cr3, eax

    ; Enable Long mode
    mov     ecx,0C0000080h          ; EFER MSR 
    rdmsr 
    or      eax, 1 >> 8             ; enable long mode 
    wrmsr 

    ; Enable paging
    mov     eax,cr0 
    or      eax,1 << 31 
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


*/