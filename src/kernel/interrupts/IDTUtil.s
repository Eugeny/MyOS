global _IDT_Flush
_IDT_Flush:
    mov rax, rdi
    lidt [rax]
    sti
    ret
