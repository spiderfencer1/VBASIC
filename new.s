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
fun_Main:
 enter 0,0
 ; input statement.
 mov eax,1
 sub esp,8
 push ebx
 push pattern
 call __isoc99_scanf
 add esp,16
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call printf
 add esp,12
 mov esp,[esp]
 add esp,4
 ; variable.
 push dword [ebp-4]
 ; return statement.
 pop eax
 leave
 ret
