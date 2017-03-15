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
fun_IsPrime:
 enter 0,0
 and esp,0xfffffff0
 sub esp,4
 ; assignment.
 pop eax
 ; while statement.
 ; return statement.
 pop eax
 leave
 ret
fun_Main:
 enter 0,0
 and esp,0xfffffff0
 sub esp,8
 ; input statement.
 ; assignment.
 pop eax
 ; while statement.
 ; return statement.
 pop eax
 leave
 ret
