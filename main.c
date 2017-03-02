#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void error(const char* fmt, ...)
{
 printf("\x1b[31m");
 va_list args;
 va_start(args,fmt);
 vprintf(fmt,args);
 va_end(args);
 printf("\x1b[0m\n");
 exit(-1);
}

typedef struct
{
 int len;
 void** data;
} vec;
vec* newvec(void)
{
 vec* v = malloc(sizeof(vec));
 v->len = 0;
 v->data = malloc(0);
 return v;
}
void vecfree(vec* v) { free(v->data); free(v); }
void* vecadd(vec* v, void* e)
{
 v->data = realloc(v->data, sizeof(void*) * v->len + sizeof(e));
 return v->data[v->len++] = e;
}
void* vecget(vec* v, int i) { return (0 <= i && i < v->len) ? v->data[i] : NULL; }
void* vecset(vec* v, int i, void* e) { return (0 <= i && i < v->len) ? v->data[i] = e : NULL; }
vec* initvec(int nargs,...)
{
 vec* v = newvec();
 va_list args;
 va_start(args,nargs);
 for (int i=0;i<nargs;i++) { vecadd(v,va_arg(args,void*)); }
 return v;
}

#define tolower(chr) (('A'<=chr&&chr<='Z')?chr-26:chr)
typedef struct
{
 char* type, *lexeme;
} token;
token* newtok(char* type, char* lexeme)
{
 token* t = malloc(sizeof(token));
 strcat(strcpy((t->type = malloc(strlen(type)+1)),type),"\0");
 strcat(strcpy((t->lexeme = malloc(strlen(lexeme)+1)),lexeme),"\0");
 return t;
}
void tokfree(token* t) { free(t->type); free(t->lexeme); free(t); }

#define DIGIT(chr) ('0'<=chr&&chr<='9')
#define IDENT(chr) (('A'<=chr&&chr<='z')||chr=='_')
vec* tokenize(char* buf)
{
 vec* tokens = newvec();
 int pos = 0;

 char* keywords[] = 
 {
  "Function","End","Integer","As","Dim","Let","Return",
  "If","Then","Else","Do","While","Loop","Print","Input"
 };

 while (pos < strlen(buf))
 {
  char look = buf[pos++];

  if (look == '\n')
  {
   int count = 1;
   for(;pos<strlen(buf)&&buf[pos]=='\n';count++,pos++);
   char* nl=malloc(count+1);
   for(int i=0;i<count;i++){nl[i]='n';}
   nl[count]=0;
   vecadd(tokens,newtok("newline",nl));
  }

  int flg = 0;
  for (int i=0;i<sizeof(keywords)/sizeof(char*);i++)
  {
   if (strncmp(&buf[pos-1],keywords[i],strlen(keywords[i])) == 0)
   {
    pos += strlen(keywords[i]) - 1;
    vecadd(tokens,newtok(keywords[i],keywords[i]));
    flg = 1;
    break;
   }
  }
  if (flg) { continue; }
  else if (DIGIT(look))
  {
   char* num = malloc(1);
   num[0] = look;
   int i = 1;
   while (pos < strlen(buf) && DIGIT(buf[pos]))
   {
    num = realloc(num,i+1);
    num[i++] = buf[pos++];
   }
   num = realloc(num,i+1);
   num[i] = 0;
   vecadd(tokens,newtok("num",num));
   free(num);
  }
  else if (IDENT(look))
  {
   char* var = malloc(1);
   var[0] = look;
   int i = 1;
   while (pos < strlen(buf) && (DIGIT(buf[pos]) || IDENT(buf[pos])))
   {
    var = realloc(var,i+1);
    var[i++] = buf[pos++];
   }
   var = realloc(var,i+1);
   var[i] = 0;
   vecadd(tokens,newtok("var",var));
   free(var);
  }
  else
  {
   switch (look)
   {
    case '+':
     vecadd(tokens,newtok("plus","+"));
     break;
    case '-':
     vecadd(tokens,newtok("minus","-"));
     break;
    case '*':
     vecadd(tokens,newtok("mult","*"));
     break;
    case '/':
     vecadd(tokens,newtok("div","/"));
     break;
    case '(':
     vecadd(tokens,newtok("lparen","("));
     break;
    case ')':
     vecadd(tokens,newtok("rparen",")"));
     break;
    case '<':
     if (pos < strlen(buf) && buf[pos] == '=') { pos++; vecadd(tokens,newtok("lte","<=")); break; }
     vecadd(tokens,newtok("ltn","<"));
     break;
    case '>':
     if (pos < strlen(buf) && buf[pos] == '=') { pos++; vecadd(tokens,newtok("gte",">=")); break; }
     vecadd(tokens,newtok("gtn",">"));
     break;
    case '=':
     vecadd(tokens,newtok("equal","="));
     break;
    case ',':
     vecadd(tokens,newtok("comma",","));
     break;
    default:
     break;
   }
  }
 }
 token* t = (token*)(vecget(tokens,tokens->len-1));
 if (strcmp(t->type,"newline") != 0) { vecadd(tokens,newtok("newline","1")); }
 return tokens;
}

typedef enum{
 N_PROG,
 N_BLOCK,
 N_FUNC,
 N_ARG,
 N_ASSIGN,
 N_RETURN,
 N_DECL,
 N_IFS,
 N_IFE,
 N_WHILE,
 N_PRINT,
 N_INPUT,
 N_BINARY,
 N_NEG,
 N_CONST,
 N_CALL,
 N_VAR
}node_type;

typedef struct{node_type ntype;}                                          n_node;
typedef struct{node_type ntype;vec* body;}                                n_prog;
typedef struct{node_type ntype;vec* values,*ops;}                         n_binary;
typedef struct{node_type ntype;long val;}                                 n_const;
typedef struct{node_type ntype;char* name;int index;}                     n_var;
typedef struct{node_type ntype;n_node* rval;}                             n_neg;
typedef struct{node_type ntype;char* name;vec* args;}                     n_call;
typedef struct{node_type ntype;vec* body;}                                n_block;
typedef struct{node_type ntype;char* name,*type;vec* args;n_block* body;} n_func;
typedef struct{node_type ntype;char* name,*type;}                         n_arg;
typedef struct{node_type ntype;char* name;n_node* rval;}                  n_assign;
typedef struct{node_type ntype;n_node* rval;}                             n_return;
typedef struct{node_type ntype;char* name,*type;}                         n_decl;
typedef struct{node_type ntype;n_node* cond;n_block* body;}               n_ifs;
typedef struct{node_type ntype;n_node* cond;n_block* t,*f;}               n_ife;
typedef struct{node_type ntype;n_node* cond;n_block* body;}               n_while;
typedef struct{node_type ntype;n_node* rval;}                             n_print;
typedef struct{node_type ntype;char* name;}                               n_input;

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
 while(lmatch("=",tokens,p) ||lmatch("<>",tokens,p)
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

typedef struct {
 vec* keys,*data;
 int len;
} map;

map* newmap(void){
 map* m=malloc(sizeof(map));
 m->keys=newvec();
 m->data=newvec();
 m->len=0;
 return m;
}

void* mapget(map* m,char* key){
 for(int i=0;i<m->keys->len;i++){
  if(strcmp((char*)vecget(m->keys,i),key)==0){
   return vecget(m->data,i);
  }
 }
 return NULL;
}

void* mapset(map* m,char* key,void* val){
 for(int i=0;i<m->keys->len;i++){
  if(strcmp((char*)vecget(m->keys,i),key)==0){
   return vecset(m->data,i,val);
  }
 }
 m->len++;
 vecadd(m->keys,key);
 return vecadd(m->data,val);
}

typedef struct{
 char* name,*type;
 int idx;
}fnvar;

typedef struct{
 char* name;
 vec* args; // vec<fnvar*> 
 vec* vars; // vec<fnvar*>
}fntempl;

vec* make_function_vec(n_prog* n){
 vec* v = newvec();
 for(int i=0;i<n->body->len;i++){
  n_func* f=(n_func*)vecget(n->body,i);
  fntempl* ft=malloc(sizeof(fntempl));
  ft->name = f->name;
  ft->args = newvec();
  for(int i=0;i<f->args->len;i++)
  {
   n_arg* a=(n_arg*)vecget(f->args,i);
   fnvar* fv=malloc(sizeof(fnvar));
   strcat(strcpy((fv->type = malloc(strlen(a->type)+1)),a->type),"\0");
   strcat(strcpy((fv->name = malloc(strlen(a->name)+1)),a->name),"\0");
   fv->idx = i;
   vecadd(ft->args,fv);
  }
  ft->vars = newvec();
  for(int i=0;i<f->body->body->len;i++)
  {
   n_node* n=(n_node*)vecget(f->body->body,i);
   if(n->ntype!=N_DECL){continue;}
   n_decl* d=(n_decl*)n;
   fnvar* fv=malloc(sizeof(fnvar));
   strcat(strcpy((fv->type = malloc(strlen(d->type)+1)),d->type),"\0");
   strcat(strcpy((fv->name = malloc(strlen(d->name)+1)),d->name),"\0");
   fv->idx = i;
   vecadd(ft->vars,fv);
  }
  vecadd(v,ft);
 }
 return v;
}

typedef struct
{
 int debug_file;
 int debug_lexer;
 int debug_parser;
 int repl;
 vec* pre; // vec<char*>
 vec* filenames; // vec<char*>
} opts;

opts* parse_opts(int argc, char** argv)
{
 opts* o = malloc(sizeof(opts));
 o->debug_file = 0;
 o->debug_lexer = 0;
 o->debug_parser = 0;
 o->repl = 0;
 o->pre = newvec();
 o->filenames = newvec();
 for(int i=1;i<argc;i++)
 {
  if(strcmp(argv[i],"-dfile") == 0) { o->debug_file = 1; }
  else if(strcmp(argv[i],"-dlex") == 0
       || strcmp(argv[i],"-dlexer") == 0) { o->debug_lexer = 1; }
  else if(strcmp(argv[i],"-dparse") == 0
       || strcmp(argv[i],"-dparser") == 0) { o->debug_parser = 1; }
  else if(strcmp(argv[i],"-exec") == 0)
  {
   vecadd(o->pre,argv[++i]);
  }
  else if(strcmp(argv[i],"-i") == 0
       || strcmp(argv[i],"-repl") == 0) { o->repl = 1; }
  else
  {
   vecadd(o->filenames,argv[i]);
  }
 }
 return o;
}

void generate_block(n_block* n)
{
 for(int i=0;i<n->body->len;i++)
 {
  n_node* stmt=(n_node*)vecget(n->body,i);
  switch(stmt->ntype)
  {
   case N_LET:
    generate_let((n_assign*)stmt);
   break;
   case N_RETURN:
    generate_return((n_return*)stmt);
   break;
   case N_DIM:
   break;
   case N_IF:
    generate_if((n_return*)stmt);
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
  /*
  if     (lmatch("Let",tokens,p)){vecadd(b->body,parse_let(tokens,p));}
  else if(lmatch("Return",tokens,p)){vecadd(b->body,parse_return(tokens,p));}
  else if(lmatch("Dim",tokens,p)){vecadd(b->body,parse_decl(tokens,p));}
  else if(lmatch("If",tokens,p)){vecadd(b->body,parse_if(tokens,p));}
  else if(lmatch("Do",tokens,p)){vecadd(b->body,parse_while(tokens,p));}
  else if(lmatch("Print",tokens,p)){vecadd(b->body,parse_print(tokens,p));}
  else if(lmatch("Input",tokens,p)){vecadd(b->body,parse_input(tokens,p));}
  */
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

void exec(char* buf,opts* o)
{
 vec* tokens = tokenize(buf);
 if(o->debug_lexer)
 {
  for(int i=0;i<tokens->len;i++) {
   token* t=(token*)vecget(tokens,i);
   printf("\x1b[32m{`%s`,`%s`}\x1b[0m\n",t->type,t->lexeme);
  }
 }
 n_prog* n = parse(tokens);
 if(o->debug_parser)
 {
  printf("\x1b[32m");
  print_node((n_node*)n,0);
  printf("\x1b[0m");
 }
 vec* functions = make_function_vec(n);
 generate(n,functions);
 for (int i=0;i<tokens->len;i++) { tokfree((token*)vecget(tokens,i)); }
 vecfree(tokens);
}

#define USAGE(name,cond) if (cond) { error("\x1b[32mUsage: %s <filename>\x1b[0m\n",name); exit(-1); }
int main(int argc, char** argv)
{
 USAGE(argv[0],argc < 2);
 int debug = 0;
 opts* o = parse_opts(argc,argv);
 if(o->repl)
 {
  while(1)
  {
   printf("≈> ");
   char* line = malloc(0);
   int c,i = 1;
   while((c = getchar()) != '\n') { line = realloc(line,i); line[i-1] = c; i++; }
   exec(line,o);
   free(line);
  }
 }
 else
 {
  USAGE(argv[0],o->filenames->len + o->pre->len < 1);
  char* buf = malloc(0);
  long len = 0;
  for(int i=0;i<o->pre->len;i++)
  {
   char* text = (char*)vecget(o->pre,i);
   long i = len;
   len += strlen(text);
   buf = realloc(buf,len);
   strncpy(&buf[i],text,strlen(text));
  }
  for(int i=0;i<o->filenames->len;i++)
  {
   char* fl = (char*)vecget(o->filenames,i);
   FILE* fp = fopen(fl,"r");
   USAGE(argv[0],fp == NULL);
   fseek(fp,0,SEEK_END);
   long llen = ftell(fp);
   fseek(fp,0,SEEK_SET);
   long i = len;
   len += llen;
   buf = realloc(buf,len);
   fread(&buf[i],1,llen,fp);
   fclose(fp);
  }
  buf[len] = 0;
  if(o->debug_file) { printf("\x1b[32m%s\x1b[0m\n",buf); }
  exec(buf,o);
  free(buf);
 }
 return 0;
}
