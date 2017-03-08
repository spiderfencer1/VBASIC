#include <stdio.h>

#include "../include/error.h"
#include "../include/generator.h"
#include "../include/parser.h"
#include "../include/vec.h"

void generate_let(n_assign* a)
{
 printf("; assignment.\npop eax\n");
}

void generate_return(n_return* r)
{
 printf("; return statement.\npop eax\nleave\nret\n"); 
}

void generate_if(n_ifs* i)
{
 printf("; if statement.\n");
}

void generate_while(n_while* w)
{
 printf("; while statement.\n");
}

void generate_print(n_print* p)
{
 printf("; print statement.\n");
}

void generate_input(n_input* i)
{
 printf("; input statement.\n");
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
 printf(" and esp,0xfffffff0\n");
 printf(" sub esp,%d\n",4*f->vars->len);
 generate_block(nf->body);
 printf(" leave\n");
 printf(" ret\n");
}

void generate(n_prog* n,vec* fncs)
{
 printf("global _main\n"
 "\n"
 "extern _scanf\n"
 "extern _printf\n"
 "\n"
 "\n"
 "section .data\n"
 "\n"
 "    pattern: db \"%%d\",0\n"
 "    message: db \"%%d\",10,0\n"
 "\n"
 "\n"
 "section .text\n"
 "\n"
 "_main:\n"
 "    jmp fun_Main\n");
 for(int i=0;i<fncs->len;i++)
 {
  fntempl* f = (fntempl*)vecget(fncs,i);
  n_func* nf=(n_func*)vecget(n->body,i);
  generate_funct(f,nf);
 }
}
