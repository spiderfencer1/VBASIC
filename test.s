global main
section .bss
 input: resd 1
extern scanf
extern printf
section .data
 pattern: db "%d",0
 message: db "%d",10,0
section .text
main:
 jmp fun_Main
fun_IsPrime:
 enter 0,0
 ; assignment Divisor.
 ; while statement.
 ; constant.
 push dword 1
 ; return statement.
 pop eax
 leave
 ret
fun_Main:
 enter 0,0
 ; input statement.
 mov eax,1
 sub esp,8
 push ebx
 push pattern
 call scanf
 add esp,16
 ; assignment N.
 ; while statement.
 ; constant.
 push dword 0
 ; return statement.
 pop eax
 leave
 ret
