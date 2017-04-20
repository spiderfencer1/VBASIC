#include <stdio.h>
#include <string.h>

#include "../include/error.h"
#include "../include/generator.h"
#include "../include/parser.h"
#include "../include/vec.h"

void generate_const(n_const* c)
{
 printf(" ; constant.\n");
 printf(" push dword %ld\n",c->val);
}

void generate_var(n_var* v)
{
 printf(" ; variable.\n");
 //printf(" push ")
}

void generate_bin(n_binary* b)
{
 printf(" ; binary expression.\n");
 n_node* a = (n_node*)vecget(b->values,0);
 if(a->ntype == N_CONST) { generate_const((n_const*)a); }
 else if(a->ntype == N_VAR) { generate_var((n_var*)a); }
 else if(a->ntype == N_BINARY) { generate_bin((n_binary*)a); }
 for(int i=0;i<b->ops->len;i++)
 {
  n_node* c = (n_node*)vecget(b->values,i+1);
  if(c->ntype == N_CONST) { generate_const((n_const*)c); }
  else if(c->ntype == N_VAR) { generate_var((n_var*)c); }
  else if(c->ntype == N_BINARY) { generate_bin((n_binary*)c); }
  char* op = (char*)vecget(b->ops,i);
  if(strcmp(op,"+") == 0)
  {
   printf(" pop eax\n");
   printf(" add [esp],eax\n");
  }
  else if(strcmp(op,"-") == 0)
  {
   printf(" pop eax\n");
   printf(" sub [esp],eax\n");
  }
  else if(strcmp(op,"*") == 0)
  {
   printf(" mov edx,0\n");
   printf(" pop eax\n");
   printf(" imul dword [esp]\n");
   printf(" mov [esp],eax\n");
  }
  else if(strcmp(op,"/") == 0)
  {
   printf(" mov edx,0\n");
   printf(" mov eax,[esp+4]\n");
   printf(" idiv dword [esp]\n");
   printf(" add esp,4\n");
   printf(" mov [esp],eax\n");
  }
  else
  {
   error("Unsupported operator: %s\n",op);
  }
 }
}

void generate_let(n_assign* a)
{
 printf(" ; assignment.\n"
 " pop eax\n");
}

void generate_return(n_return* r)
{
 if(r->rval->ntype == N_CONST) { generate_const((n_const*)r->rval); }
 else if(r->rval->ntype == N_VAR) { generate_var((n_var*)r->rval); }
 else if(r->rval->ntype == N_BINARY) { generate_bin((n_binary*)r->rval); }
 printf(" ; return statement.\n"
 " pop eax\n"
 " leave\n"
 " ret\n"
 );
}

void generate_if(n_ifs* i)
{
 printf(" ; if statement.\n"

 );
}

void generate_while(n_while* w)
{
 printf(" ; while statement.\n"

 );
}

void generate_print(n_print* p)
{
 if(p->rval->ntype == N_BINARY){
  generate_bin((n_binary*)p->rval);
 }else if(p->rval->ntype == N_CONST){
  generate_const((n_const*)p->rval);
 }
 printf(" ; print statement.\n"
 "mov eax,[esp]\n"
 "mov ebx,esp\n"
 "and esp,0xfffffff0\n"
 "push ebx\n"
 "sub esp,4\n"
 "push eax\n"
 "push message\n"
 "mov eax,1\n"
 "call _printf\n"
 "add esp,12\n"
 "mov esp,[esp]\n"
 "add esp,4\n"
 );
}

void generate_input(n_input* i)
{
 printf(" ; input statement.\n"
 " mov eax,1\n"
 " sub esp,8\n"
 " push ebx\n"
 " push pattern\n"
 " call _scanf\n"
 " add esp,16\n"
 );
}

void generate_block(n_block* n)
{
 for(int i=0;i<n->body->len;i++)
 {
  n_node* stmt=(n_node*)vecget(n->body,i);
  switch(stmt->ntype)
  {
   case N_ASSIGN:
    generate_let((n_assign*)stmt);
   break;
   case N_RETURN:
    generate_return((n_return*)stmt);
   break;
   case N_DECL:
   break;
   case N_IFS:
    generate_if((n_ifs*)stmt);
   break;
   case N_WHILE:
    generate_while((n_while*)stmt);
   break;
   case N_PRINT:
    generate_print((n_print*)stmt);
   break;
   case N_INPUT:
    generate_input((n_input*)stmt);
   break;
   default:
    error("Unknown case. %d\n",stmt->ntype);
   break;
  }
 }
}

void generate_funct(fntempl* f,n_func* nf)
{
 printf("fun_%s:\n",f->name);
 printf(" enter 0,0\n");
 generate_block(nf->body);
}

void generate(n_prog* n,vec* fncs)
{
 printf("global _main\n"
 "section .bss\n"
 " input: resd 1\n"
 "extern _scanf\n"
 "extern _printf\n"
 "section .data\n"
 " pattern: db \"%%d\",0\n"
 " message: db \"%%d\",10,0\n"
 "section .text\n"
 "_main:\n"
 " jmp fun_Main\n");
 for(int i=0;i<fncs->len;i++)
 {
  fntempl* f = (fntempl*)vecget(fncs,i);
  n_func* nf=(n_func*)vecget(n->body,i);
  generate_funct(f,nf);
 }
}
