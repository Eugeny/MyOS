global CPUSaveState
CPUSaveState:
    mov ecx, [esp+4]
    mov eax, [esp]

    mov [ecx], eax
    mov [ecx+4], esp
    mov [ecx+8], ebp
    mov [ecx+12], ebx
    mov [ecx+16], edi
    mov [ecx+20], esi

    ret


global CPURestoreState
CPURestoreState:
    mov ecx, [esp+8] ; State
    mov eax, [esp+4] ; AS

    mov edx, [ecx]
    mov esp, [ecx+4]
    mov ebp, [ecx+8]
    mov ebx, [ecx+12]
    mov edi, [ecx+16]
    mov esi, [ecx+20]
    mov cr3, eax
    add esp, 4

    jmp edx
