#include <stdio.h>
#include <string.h>

#include "../include/error.h"
#include "../include/generator.h"
#include "../include/parser.h"
#include "../include/sym_table.h"
#include "../include/sym_table_node.h"
#include "../include/vec.h"

int if_stmt_count = 0;
int while_stmt_count = 0;
sym_table* st;

void generate_const(n_const* c)
{
 printf(" ; constant.\n");
 printf(" push dword %ld\n",c->val);
}

void get_stack_offset(int offset){
 if(offset >= 0){
  printf(" [ebp+%d]",offset);
 }
 printf(" [ebp%d]",offset);
}

void generate_var(n_var* v)
{
 printf(" ; variable.\n");
 sym_table_node* stn = (sym_table_node*)symtableget(st,v->name);
 if(stn == NULL){
  error("Undeclared variable: %s.\n",v->name);
 }
 printf(" push dword");
 get_stack_offset(stn->offset);
 printf("\n");
}

void generate_dim(n_decl* d){
 symtabledecl(st,d->name,NUM_LOCAL); 
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
		else if(strcmp(op,"==") == 0 || strcmp(op,">=") == 0 || strcmp(op,"<=") == 0 ||
				strcmp(op,">") == 0 || strcmp(op,"<") == 0 || strcmp(op,"!=") == 0){
			printf(
			" pop ebx\n"
			" pop eax\n"
			" cmp eax,ebx\n"
			);
		}
  else
  {
   error("Unsupported operator: %s\n",op);
  }
 }
}

void generate_value(n_node* n){
 if(n->ntype == N_CONST){generate_const((n_const*)n);}
 else if(n->ntype == N_BINARY){generate_bin((n_binary*)n);}
 else if(n->ntype == N_VAR){generate_var((n_var*)n);}
}

void generate_let(n_assign* a)
{
 generate_value((n_node*)a->rval);
 printf(" ; assignment %s.\n pop dword",a->name);
 sym_table_node* stn = symtableget(st,a->name);
 if(stn == NULL){
  error("Undeclared variable: %s.\n",a->name);
  return;
 }
 get_stack_offset(stn->offset);
 printf("\n");
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

void generate_jump_cond(n_binary* cond,int label_no,char* label_type){
	generate_bin(cond);	
	char* op = vecget(cond->ops,0);
	if(strcmp(op,"==") == 0){printf(" jne");}
	else if(strcmp(op,"!=") == 0){printf(" je");}
	else if(strcmp(op,"<") == 0){printf(" jge");}
	else if(strcmp(op,">") == 0){printf(" jle");}
	else if(strcmp(op,"<=") == 0){printf(" jg");}
	else if(strcmp(op,">=") == 0){printf(" jl");}
	else{error("Unsupported operator: %s\n",op);}
	printf(" %s_%d\n",label_type,label_no);
}

void generate_if(n_node* n)
{
	int label_no = ++if_stmt_count;
 printf(" ; if statement.\n");
	generate_jump_cond((n_binary*)(((n_ifs*)n)->cond),label_no,"_if_stmt_false");
	if(n->ntype == N_IFE){
		generate_block(((n_ife*)n)->t);
	}else{
		generate_block(((n_ifs*)n)->body);
	}
	printf(" jmp _if_stmt_end_%d\n",label_no);
	printf("_if_stmt_false_%d: \n",label_no);
	if(n->ntype == N_IFE){
		generate_block(((n_ife*)n)->f);
	}
	printf("_if_stmt_end_%d: \n",label_no);
}

void generate_while(n_while* w)
{
 int label_no = ++while_stmt_count;
 printf(" ; while statement.\n_while_stmt_start_%d",label_no);
 generate_jump_cond((n_binary*)(((n_while*)w)->cond),label_no,"_while_stmt_end");
 generate_block(w->body);
 generate_jump_cond((n_binary*)(((n_while*)w)->cond),label_no,"_while_stmt_end");
 printf(" jmp _while_stmt_start_%d",label_no);
}

void generate_print(n_print* p){
 if(p->rval->ntype == N_BINARY){
  generate_bin((n_binary*)p->rval);
 }else if(p->rval->ntype == N_CONST){
  generate_const((n_const*)p->rval);
 }else if(p->rval->ntype == N_VAR){
  printf(" push dword");
  n_var* v = (n_var*)(p->rval);
  sym_table_node* stn = symtableget(st,v->name);
  if(stn == NULL){
   error("Undeclared variable: %s",v->name);
  }
  get_stack_offset(stn->offset);
  printf("\n");
 }
 printf(" ; print statement.\n"
 " mov eax,[esp]\n"
 " mov ebx,esp\n"
 " and esp,0xfffffff0\n"
 " push ebx\n"
 " sub esp,4\n"
 " push eax\n"
 " push message\n"
 " mov eax,1\n");
#ifdef __MACH__
  printf(" call _printf\n");
#else
  printf(" call printf\n");
#endif
 printf(
 " add esp,12\n"
 " mov esp,[esp]\n"
 " add esp,4\n"
 );
}

void generate_input(n_input* i)
{
 printf(" ; input statement.\n"
 " mov eax,1\n"
 " sub esp,8\n"
 " push ebx\n"
 " push pattern\n");
#ifdef __MACH__
  printf(" call _scanf\n");
#else
  printf(" call __isoc99_scanf\n");
#endif
 printf(" add esp,16\n");
 printf(" mov");
 sym_table_node* stn = symtableget(st,i->name);
 if(stn == NULL){
  error("Undeclared variable: %s",i->name);
 }
 get_stack_offset(stn->offset);
 printf(",ebx\n");
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
    generate_dim((n_decl*)stmt);
   break;
   case N_IFS:
			case N_IFE:
    generate_if(stmt);
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
 st = newsymtable();
 for(int i=0;i<f->args->len;i++){
  symtabledecl(st,((fnvar*)vecget(f->args,i))->name,NUM_PARAM);
 }
 generate_block(nf->body);
}

void generate(n_prog* n,vec* fncs)
{
#ifdef __MACH__
  printf("global _main\n");
#else
  printf("global main\n");
#endif
 printf("section .bss\n"
 " input: resd 1\n");
#ifdef __MACH__
  printf("extern _scanf\n"
  "extern _printf\n"
  );
#else
  printf("extern __isoc99_scanf\n"
  "extern printf\n"
  );
#endif
 printf("section .data\n"
 " pattern: db \"%%d\",0\n"
 " message: db \"%%d\",10,0\n"
 "section .text\n");
#ifdef __MACH__
  printf("_main:\n");
#else
  printf("main:\n");
#endif
 printf(" jmp fun_Main\n");
 for(int i=0;i<fncs->len;i++)
 {
  fntempl* f = (fntempl*)vecget(fncs,i);
  n_func* nf=(n_func*)vecget(n->body,i);
  generate_funct(f,nf);
 }
}
