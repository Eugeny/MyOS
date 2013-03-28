CODE_SELECTOR equ 2 << 3                 ; 32-bit code selector (ring 0) 
RCODE_SELECTOR equ 7 << 3  
RDATA_SELECTOR equ 8 << 3  

global ApmPoweroff
ApmPoweroff:
bits 64
    cli

    push    CODE_SELECTOR
    mov     rcx, exiting_64
    push    rcx
    o64 retf

bits 32
exiting_64:
    ; Disable paging
    mov     eax, cr0 
    btc     eax, 31
    mov     cr0, eax

    ; Disable long mode
    mov     ecx, 0c0000080h
    rdmsr 
    btc     eax, 8 
    wrmsr 

    ; Recover IDT
    extern  realmode_idt_backup
    extern  realmode_idtr
    mov     eax, realmode_idt_backup
    mov     [realmode_idtr], eax
    lidt    [realmode_idtr]

    ; Leaving PM
    jmp     RCODE_SELECTOR:exiting_pm

exiting_pm:
    mov     ax, RDATA_SELECTOR
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax

    ; Disable PM    
    mov     eax, cr0
    btc     eax, 0
    mov     cr0, eax

    jmp     0:pm_exit

bits 16
pm_exit:
    ; REAL MODE --------------------
    mov     ax, 0
    mov     ds, ax
    mov     ss, ax
    mov     ax, rm_stack_top
    mov     sp, ax

    mov     ax, 5301h ; activate real-mode APM interface
    xor     bx, bx
    int     15h 

    mov     ax, 530eh ; Select APM version
    xor     bx, bx
    mov     cx, 0102h ; v1.2
    int     15h 

    mov     ax, 530fh ; Activate APM for every devices
    mov     bx, 0001h ; ID 1=everyl devices
    mov     cx, 0001h ; 1=engage
    int     15h 

    mov     ax, 5308h ; Atomatically activate APM for every devices
    mov     bx, 0001h ; ID 1=every device
    mov     cx, 0001h ; 1=enable
    int     15h 

    mov     ax, 5307h ; set device state
    mov     bx, 0001h ; ID 1=every device
    mov     cx, 0003h ; status 3=Shutdown
    int     15h 
    jmp     $

rm_stack: times 1024 db 0    
rm_stack_top: