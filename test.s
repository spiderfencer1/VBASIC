global main
section .bss
 input: resd 1
extern __isoc99_scanf
extern printf
section .data
 pattern: db "%d",0
 message: db "%d",10,0
section .text
main:
 jmp fun_Main
fun_IsPrime:
 enter 0,0
 ; constant.
 push dword 2
 ; assignment Divisor.
 pop dword [ebp-4]
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
 call __isoc99_scanf
 add esp,16
 ; constant.
 push dword 2
 ; assignment N.
 pop dword [ebp-4]
 ; while statement.
 ; constant.
 push dword 0
 ; return statement.
 pop eax
 leave
 ret
