global fork
fork:
    mov eax, 1
    mov ecx, forkres
    int 0x80
    mov eax, [forkres]
    ret
forkres:
    dd 0


global newThread
newThread:
    mov eax, 2
    mov ebx, [esp+4]
    mov edx, [esp+8]
    mov ecx, newThreadres
    int 0x80
    mov eax, [newThreadres]
    ret
newThreadres:
    dd 0
