global CPUSwitchContext
CPUSwitchContext:
    mov ecx, [esp+12]
    mov eax, [esp]

    mov [ecx], eax
    mov [ecx+4], esp
    mov [ecx+8], ebp
    mov [ecx+12], ebx
    mov [ecx+16], edi
    mov [ecx+20], esi

    mov ecx, [esp+4] ; State
    mov eax, [esp+8] ; AS

    mov edx, [ecx]
    mov esp, [ecx+4]
    mov ebp, [ecx+8]
    mov ebx, [ecx+12]
    mov edi, [ecx+16]
    mov esi, [ecx+20]

    mov cr3, eax
    ret
    add esp, 12

    jmp edx
