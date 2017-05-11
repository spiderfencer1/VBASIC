#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../include/error.h"
#include "../include/parser.h"

void iprintf(int i,const char* fmt,...){
 for(int j=0;j<i;j++){printf(" ");}
 va_list args;
 va_start(args,fmt);
 vprintf(fmt,args);
 va_end(args);
}

void print_node(n_node* n,int depth) {
 switch(n->ntype){
  case N_PROG:
   {
    iprintf(depth,"n_prog\n");
    n_prog* p=(n_prog*)n;
    for(int i=0;i<p->body->len;i++){
     print_node((n_node*)vecget(p->body,i),depth+1);
    }
   }
  break;
  case N_BINARY:
   {
    iprintf(depth,"n_binary\n");
    n_binary* b=(n_binary*)n;
    for(int i=0;i<b->values->len+b->ops->len;i++){
     if(i%2==0){
      print_node((n_node*)vecget(b->values,i),depth+1);
     }else{
      iprintf(depth+1,"%s",(char*)vecget(b->values,i));
     }
    }
   }
  break;
  case N_CONST:
   iprintf(depth,"n_const\n");
   iprintf(depth+1,"%d\n",((n_const*)n)->val);
  break;
  case N_VAR:
   iprintf(depth,"n_var\n");
   iprintf(depth+1,"%s\n",((n_var*)n)->name);
  break;
  case N_NEG:
   iprintf(depth,"n_neg\n");
   print_node((n_node*)(((n_neg*)n)->rval),depth+1);
  break;
  case N_CALL:
   {
    iprintf(depth,"n_call\n");
    n_call* c=(n_call*)n;
    iprintf(depth+1,"%s\n",c->name);
    for(int i=0;i<c->args->len;i++){
     print_node((n_node*)vecget(c->args,i),depth+1);
    }
   }
  break;
  case N_BLOCK:
   {
    iprintf(depth,"n_block\n");
    n_block* b=(n_block*)n;
    for(int i=0;i<b->body->len;i++){
     print_node((n_node*)vecget(b->body,i),depth+1);
    }
   }
  break;
  case N_FUNC:
   {
    iprintf(depth,"n_func\n");
    n_func* f=(n_func*)n;
    iprintf(depth+1,"%s\n",f->name);
    iprintf(depth+1,"%s\n",f->type);
    for(int i=0;i<f->args->len;i++) {
     print_node((n_node*)vecget(f->args,i),depth+1);
    }
    print_node((n_node*)f->body,depth+1);
   }
  break;
  case N_ARG:
   {
    iprintf(depth,"n_arg\n");
    n_arg* a=(n_arg*)n;
    iprintf(depth+1,"%s\n",a->name);
    iprintf(depth+1,"%s\n",a->type);
   }
  break;
  case N_ASSIGN:
   {
    iprintf(depth,"n_assign\n");
    n_assign* a=(n_assign*)n;
    iprintf(depth+1,"%s\n",a->name);
    print_node((n_node*)a->rval,depth+1);
   }
  break;
  case N_RETURN:
   {
    iprintf(depth,"n_return\n");
    n_return* r=(n_return*)n;
    print_node((n_node*)r->rval,depth+1);
   }
  break;
  case N_DECL:
   {
    iprintf(depth,"n_decl\n");
    n_decl* d=(n_decl*)n;
    iprintf(depth+1,"%s\n",d->name);
    iprintf(depth+1,"%s\n",d->type);
   }
  break;
  case N_IFS:
   {
    iprintf(depth,"n_ifs\n");
    n_ifs* i=(n_ifs*)n;
    print_node((n_node*)i->cond,depth+1);
    print_node((n_node*)i->body,depth+1);
   }
  break;
  case N_IFE:
   {
    iprintf(depth,"n_ife\n");
    n_ife* i=(n_ife*)n;
    print_node((n_node*)i->cond,depth+1);
    print_node((n_node*)i->t,depth+1);
    print_node((n_node*)i->f,depth+1);
   }
  break;
  case N_WHILE:
   {
    iprintf(depth,"n_while\n");
    n_while* w=(n_while*)n;
    print_node((n_node*)w->cond,depth+1);
    print_node((n_node*)w->body,depth+1);
   }
  break;
  case N_PRINT:
   iprintf(depth,"n_print\n");
   print_node((n_node*)(((n_print*)n)->rval),depth+1);
  break;
  case N_INPUT:
   iprintf(depth,"n_input\n");
   iprintf(depth+1,"%s\n",((n_input*)n)->name);
  break;
  default:
  break;
 }
}

token* fetch(vec* tokens,int* p){return(*p<tokens->len)?(token*)vecget(tokens,*p):newtok("eof","eof");}
int tmatch(char* type,vec* tokens,int* p){return strcmp(type,fetch(tokens,p)->type)==0;}
int lmatch(char* lexeme,vec* tokens,int* p){return strcmp(lexeme,fetch(tokens,p)->lexeme)==0;}
void texpect(char* type,vec* tokens,int* p){
 if(!tmatch(type,tokens,p)){error("Expected `%s`,got `%s`.",type,fetch(tokens,p)->type);}
}
void lexpect(char* lexeme,vec* tokens,int* p){
 if(!lmatch(lexeme,tokens,p)){error("Expected `%s`,got `%s`.",lexeme,fetch(tokens,p)->lexeme);}
}
void texpectm(char* type,vec* tokens,int* p){
 texpect(type,tokens,p);
 (*p)++;
}
void lexpectm(char* lexeme,vec* tokens,int* p){
 lexpect(lexeme,tokens,p);
 (*p)++;
}

n_node* parse_binary(vec*,int*);

n_node* parse_parens(vec* tokens,int* p){
 (*p)++;
 n_node* b=parse_binary(tokens,p);
 lexpectm(")",tokens,p);
 return b;
}

n_const* parse_const(vec* tokens,int* p){
 n_const* c=malloc(sizeof(n_const));
 c->ntype=N_CONST;
 c->val=atol(fetch(tokens,p)->lexeme);
 (*p)++;
 return c;
}

n_var* parse_var(vec* tokens,int* p){
 n_var* v=malloc(sizeof(n_var));
 v->ntype=N_VAR;
 v->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(v->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 return v;
}

n_neg* parse_neg(vec* tokens,int* p){
 (*p)++;
 n_neg* n=malloc(sizeof(n_neg));
 n->ntype=N_NEG;
 n->rval=parse_binary(tokens,p);
 return n;
}

n_call* parse_call(vec* tokens,int* p){
 n_call* c=malloc(sizeof(n_call));
 c->ntype=N_CALL;
 c->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(c->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 lexpectm("(",tokens,p);
 c->args=newvec();
 if(lmatch(")",tokens,p)){(*p)++;}
 else{
  while(1){
   vecadd(c->args,parse_binary(tokens,p));
   if(lmatch(")",tokens,p)){(*p)++;break;}
   lexpectm(",",tokens,p);
  }
 }
 return c;
}
n_node* parse_factor(vec* tokens,int* p){
 if     (tmatch("num",tokens,p)){
  n_const* n = malloc(sizeof(n_const));
  n->ntype = N_CONST;
  n->val = atol(fetch(tokens,p)->lexeme);
  (*p)++;
  return (n_node*)n;
 }
 else if(lmatch("(",tokens,p)){return (n_node*)parse_parens(tokens,p);} 
 else if(lmatch("-",tokens,p)){return (n_node*)parse_neg(tokens,p);}
 else if(tmatch("var",tokens,p)){
  if(*p+1<tokens->len&&strcmp(((token*)vecget(tokens,*p+1))->lexeme,"(")==0){
   return(n_node*)parse_call(tokens,p);
  }
  return(n_node*)parse_var(tokens,p);
 }
 error("Cannot parse node:{`%s`,`%s`}.",fetch(tokens,p)->type,fetch(tokens,p)->lexeme);
 return NULL;
}

n_node* parse_term(vec* tokens,int* p){
 n_binary* b=malloc(sizeof(n_binary));
 b->ntype = N_BINARY;
 b->values=newvec();
 b->ops=newvec();
 vecadd(b->values,parse_factor(tokens,p));
 while(lmatch("*",tokens,p)||lmatch("/",tokens,p)){
  char* op=malloc(strlen(fetch(tokens,p)->lexeme)+1);
  vecadd(b->ops,strcat(strcpy(op,fetch(tokens,p)->lexeme),"\0"));
  (*p)++;
  vecadd(b->values,parse_factor(tokens,p));
 }
 if(b->values->len == 1) { return (n_node*)vecget(b->values,0); }
 return (n_node*)b;
}

n_node* parse_comp(vec* tokens,int* p){
 n_binary* b=malloc(sizeof(n_binary));
 b->ntype = N_BINARY;
 b->values=newvec();
 b->ops=newvec();
 vecadd(b->values,parse_term(tokens,p));
 while(lmatch("+",tokens,p)||lmatch("-",tokens,p)){
  char* op=malloc(strlen(fetch(tokens,p)->lexeme)+1);
  vecadd(b->ops,strcat(strcpy(op,fetch(tokens,p)->lexeme),"\0"));
  (*p)++;
  vecadd(b->values,parse_term(tokens,p));
 }
 if(b->values->len == 1) { return (n_node*)vecget(b->values,0); }
 return (n_node*)b;
}

n_node* parse_binary(vec* tokens,int* p){
 n_binary* b=malloc(sizeof(n_binary));
 b->ntype = N_BINARY;
 b->values=newvec();
 b->ops=newvec();
 vecadd(b->values,parse_comp(tokens,p));
 while(lmatch("==",tokens,p)||lmatch("<>",tokens,p)
     ||lmatch("<=",tokens,p)||lmatch(">=",tokens,p)
     ||lmatch("<",tokens,p) ||lmatch(">",tokens,p)){
  char* op=malloc(strlen(fetch(tokens,p)->lexeme)+1);
  vecadd(b->ops,strcat(strcpy(op,fetch(tokens,p)->lexeme),"\0"));
  (*p)++;
  vecadd(b->values,parse_comp(tokens,p));
 }
 if(b->values->len == 1) { return (n_node*)vecget(b->values,0); }
 return (n_node*)b;
}

n_block* parse_block(vec* tokens,int* p);

n_assign* parse_let(vec* tokens,int* p){
 n_assign* a=malloc(sizeof(n_assign));
 a->ntype=N_ASSIGN;
 (*p)++;
 texpect("var",tokens,p);
 a->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(a->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 lexpectm("=",tokens,p);
 a->rval=parse_binary(tokens,p);
 texpectm("newline",tokens,p);
 return a;
}

n_return* parse_return(vec* tokens,int* p){
 n_return* r=malloc(sizeof(n_return));
 r->ntype=N_RETURN;
 (*p)++;
 r->rval=parse_binary(tokens,p);
 texpectm("newline",tokens,p);
 return r;
}

n_decl* parse_decl(vec* tokens,int* p){
 n_decl* d=malloc(sizeof(n_decl));
 d->ntype=N_DECL;
 (*p)++;
 texpect("var",tokens,p);
 d->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(d->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 lexpectm("As",tokens,p);
 lexpectm("Integer",tokens,p);
 d->type="Integer";
 texpectm("newline",tokens,p);
 return d;
}

n_ife* parse_ife(n_ifs* i,vec* tokens,int* p){
 n_ife* e=malloc(sizeof(n_ife));
 e->ntype=N_IFE;
 e->cond=i->cond;
 e->t=i->body;
 (*p)++;
 texpectm("newline",tokens,p);
 e->f=parse_block(tokens,p);
 lexpectm("End",tokens,p);
 lexpectm("If",tokens,p);
 texpectm("newline",tokens,p);
 return e;
}

n_node* parse_if(vec* tokens,int* p){
 n_ifs* i=malloc(sizeof(n_ifs));
 i->ntype=N_IFS;
 (*p)++;
 i->cond=parse_binary(tokens,p);
 lexpectm("Then",tokens,p);
 texpectm("newline",tokens,p);
 i->body=parse_block(tokens,p);
 if(lmatch("Else",tokens,p)){return(n_node*)parse_ife(i,tokens,p);}
 lexpectm("End",tokens,p);
 lexpectm("If",tokens,p);
 texpectm("newline",tokens,p);
 return(n_node*)i;
}

n_while* parse_while(vec* tokens,int* p){
 n_while* w=malloc(sizeof(n_while));
 w->ntype=N_WHILE;
 (*p)++;
 lexpectm("While",tokens,p);
 w->cond=parse_binary(tokens,p);
 texpectm("newline",tokens,p);
 w->body=parse_block(tokens,p);
 lexpectm("Loop",tokens,p);
 texpectm("newline",tokens,p);
 return w;
}

n_print* parse_print(vec* tokens,int* p){
 n_print* np=malloc(sizeof(n_print));
 np->ntype=N_PRINT;
 (*p)++;
 np->rval=parse_binary(tokens,p);
 texpectm("newline",tokens,p);
 return np;
}

n_input* parse_input(vec* tokens,int* p){
 n_input* i=malloc(sizeof(n_input));
 i->ntype=N_INPUT;
 (*p)++;
 texpect("var",tokens,p);
 i->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(i->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 texpectm("newline",tokens,p);
 return i;
}

n_block* parse_block(vec* tokens,int* p){
 n_block* b=malloc(sizeof(n_block));
 b->ntype=N_BLOCK;
 b->body=newvec();
 while(1){
  if     (lmatch("Let",tokens,p)){vecadd(b->body,parse_let(tokens,p));}
  else if(lmatch("Return",tokens,p)){vecadd(b->body,parse_return(tokens,p));}
  else if(lmatch("Dim",tokens,p)){vecadd(b->body,parse_decl(tokens,p));}
  else if(lmatch("If",tokens,p)){vecadd(b->body,parse_if(tokens,p));}
  else if(lmatch("Do",tokens,p)){vecadd(b->body,parse_while(tokens,p));}
  else if(lmatch("Print",tokens,p)){vecadd(b->body,parse_print(tokens,p));}
  else if(lmatch("Input",tokens,p)){vecadd(b->body,parse_input(tokens,p));}
  else{break;}
 }
 return b;
}

n_arg* parse_arg(vec* tokens,int* p){
 n_arg* a=malloc(sizeof(n_arg));
 a->ntype=N_ARG;
 texpect("var",tokens,p);
 a->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(a->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 lexpectm("As",tokens,p);
 texpectm("Integer",tokens,p);
 a->type="Integer";
 return a;
}

n_func* parse_func(vec* tokens,int* p){
 n_func* f=malloc(sizeof(n_func));
 f->ntype=N_FUNC;
 lexpectm("Function",tokens,p);
 texpect("var",tokens,p);
 f->name=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(f->name,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 f->args=newvec();
 lexpectm("(",tokens,p);
 if(!lmatch(")",tokens,p)){
  while(1){
   vecadd(f->args,parse_arg(tokens,p));
   if(lmatch(")",tokens,p)){break;}
   lexpectm(",",tokens,p);
  }
 }
 (*p)++;
 lexpectm("As",tokens,p);
 texpect("Integer",tokens,p);
 f->type=malloc(strlen(fetch(tokens,p)->lexeme)+1);
 strcat(strcpy(f->type,fetch(tokens,p)->lexeme),"\0");
 (*p)++;
 texpectm("newline",tokens,p);
 f->body=parse_block(tokens,p);
 lexpectm("End",tokens,p);
 lexpectm("Function",tokens,p);
 texpectm("newline",tokens,p);
 return f;
}
n_prog* parse(vec* tokens){
 n_prog* n=malloc(sizeof(n_prog));
 n->ntype=N_PROG;
 n->body=newvec();
 int pos=0,*p=&pos;
 while(*p<tokens->len){vecadd(n->body,parse_func(tokens,p));}
 return n;
}
