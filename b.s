global _main
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
 ; if statement.
 ; binary expression.
 ; constant.
 push dword 0
 ; constant.
 push dword 0
 pop ebx
 pop eax
 cmp eax,ebx
 jne _if_stmt_false_1
 ; if statement.
 ; binary expression.
 ; constant.
 push dword 0
 ; constant.
 push dword 1
 pop ebx
 pop eax
 cmp eax,ebx
 jne _if_stmt_false_2
 ; constant.
 push dword 1
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
 jmp _if_stmt_end_2
_if_stmt_false_2: 
 ; if statement.
 ; binary expression.
 ; constant.
 push dword 1
 ; constant.
 push dword 1
 pop ebx
 pop eax
 cmp eax,ebx
 jl _if_stmt_false_3
 ; constant.
 push dword 2
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
 jmp _if_stmt_end_3
_if_stmt_false_3: 
 ; constant.
 push dword 3
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
_if_stmt_end_3: 
_if_stmt_end_2: 
 jmp _if_stmt_end_1
_if_stmt_false_1: 
 ; if statement.
 ; binary expression.
 ; constant.
 push dword 0
 ; constant.
 push dword 1
 pop ebx
 pop eax
 cmp eax,ebx
 jge _if_stmt_false_4
 ; constant.
 push dword 4
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
 jmp _if_stmt_end_4
_if_stmt_false_4: 
 ; constant.
 push dword 5
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
_if_stmt_end_4: 
 ; constant.
 push dword 6
 ; print statement.
 mov eax,[esp]
 mov ebx,esp
 and esp,0xfffffff0
 push ebx
 sub esp,4
 push eax
 push message
 mov eax,1
 call _printf
 add esp,12
 mov esp,[esp]
 add esp,4
_if_stmt_end_1: 
 ; constant.
 push dword 0
 ; return statement.
 pop eax
 leave
 ret
