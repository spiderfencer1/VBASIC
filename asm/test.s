; First assembly program

section .data
 helloworld: db "Hello World!",10,0

section .text

global _main
extern _printf

_main:
 enter 0,0
 and esp, 0xfffffff0
 mov eax, 0
 sub esp, 12
 push helloworld
 call _printf
 add esp, 16
 leave
 ret
