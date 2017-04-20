global _main

extern _scanf
extern _printf


section .data

    pattern: db "%d",0
    message: db "%d",10,0


section .text

_main:
    enter   0, 0
    and     esp, 0xfffffff0

    sub     esp, 8
    push    max
    push    pattern
    call    _scanf
    add     esp, 16
    mov     ebx, 0

print_number:
    mov     eax, 1
    sub     esp, 8
    push    ebx
    push    message
    call    _printf
    add     esp, 16
    inc     ebx
    cmp     ebx, [max]
    jle     print_number
    mov     eax, 0
    leave
    ret
