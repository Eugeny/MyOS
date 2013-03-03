%define COPY_STACK_SIZE qword 200
KDATA_SELECTOR equ 4 << 3  

isrq_stack times 409600 db 0
isrq_stack_top dd 0


%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

%macro PUSHAQ 0
    push rax
    push rbx
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rsi
    push rdi
    push rbp

    mov  ax, gs
    push rax
    mov  ax, fs
    push rax
%endmacro

%macro POPAQ 0
    pop rax
    mov fs, ax
    pop rax
    mov gs, ax
    pop rbp
    pop rdi
    pop rsi
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

extern isr_handler

isr_common_stub:
    PUSHAQ

    ; Copy stack onto kernel stack
    mov rax, isrq_stack_top
    sub rax, COPY_STACK_SIZE
    
    mov rsi, rsp
    mov rdi, rax
    mov rcx, COPY_STACK_SIZE
    rep movsb

    mov rdi, isrq_stack_top
    sub rdi, COPY_STACK_SIZE
    mov rsp, rdi

    call isr_handler

    POPAQ

    add rsp, 16     ; drop 2*uint64 params
    sti
    iretq


ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; IRQs
ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47

ISR_NOERRCODE 127
ISR_NOERRCODE 128
ISR_NOERRCODE 255
