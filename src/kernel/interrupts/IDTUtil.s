global _IDT_Flush
_IDT_Flush:
    mov rax, [rsp+8]
    lidt [rax]
    sti
    ret
