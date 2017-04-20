global _main
section .data
 stdin: db "%d",0
 stdout: db "%d",0
section .bss
 input: resd 1
extern _scanf
extern _printf
section .data
 pattern: db "%d",0
 message: db "%d",10,0
section .text
_main:
 jmp fun_Main
fun_Main:
 enter 0,0
 ; constant.
 push dword 0
 ; return statement.
 pop eax
 leave
 ret
