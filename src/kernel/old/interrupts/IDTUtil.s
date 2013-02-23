global _IDT_Flush
_IDT_Flush:
    mov eax, [esp+4]
    lidt [eax]
    sti
    ret
