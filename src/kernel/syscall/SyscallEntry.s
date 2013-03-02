KCODE_SELECTOR equ 3 << 3  
KDATA_SELECTOR equ 4 << 3  
UCODE_SELECTOR equ 5 << 3  
UDATA_SELECTOR equ 6 << 3  

%define MSR_STAR  0xC0000081
%define MSR_LSTAR 0xC0000082

syscall_stack times 409600 db 0
syscall_stack_top dd 0



global _syscall_init, _syscall_entry
extern _syscall_handler

_syscall_init:
    xor eax, eax
    mov edx, (KCODE_SELECTOR) + (UCODE_SELECTOR << 16)
    mov ecx, MSR_STAR
    wrmsr

    mov eax, _syscall_entry 
    xor edx, edx
    mov ecx, MSR_LSTAR
    wrmsr
    ret

_syscall_entry:
    cli 

    mov r11, rsp
    mov rbx, syscall_stack_top
    mov rsp, rbx

    push rdx
    push r10
    push r12
    push r13
    push r14
    push r15
    push rsi
    push rdi
    push rbp

    push rax
    push rcx
    push r11

    mov rdi, rsp

    call _syscall_handler

    pop r11
    pop rcx
    pop rbp    
    
    pop rbp
    pop rdi
    pop rsi
    pop r15
    pop r14
    pop r13
    pop r12
    pop r10
    pop rdx

    mov rsp, r11
    sti
    jmp rcx
    o64 sysret
