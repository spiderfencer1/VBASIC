#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#define __DEBUG__ 0

void error(const char* fmt, ...)
{
 fprintf(stderr,"\x1b[31m");
 va_list args;
 va_start(args,fmt);
 vfprintf(stderr,fmt,args);
 va_end(args);
 fprintf(stderr,"\x1b[0m\n");
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
   if (atoi(num) > INT32_MAX) { error("Integer to large %s.\n",num); }
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
    case '=':
     if (pos+1 < strlen(buf) && buf[pos+1] == '=') { pos++; vecadd(tokens,newtok("comp","==")); break; }
     vecadd(tokens,newtok("assign","="));
     break;
    case '<':
     if (pos+1 < strlen(buf) && buf[pos+1] == '=') { pos++; vecadd(tokens,newtok("lte","<=")); break; }
     vecadd(tokens,newtok("ltn","<"));
     break;
    case '>':
     if (pos+1 < strlen(buf) && buf[pos+1] == '=') { pos++; vecadd(tokens,newtok("gte",">=")); break; }
     vecadd(tokens,newtok("gtn",">"));
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
typedef struct{node_type ntype;vec* values,*ops;}                         n_binary; // done.
typedef struct{node_type ntype;long val;}                                 n_const; // done.
typedef struct{node_type ntype;char* name;}                               n_var; // done.
typedef struct{node_type ntype;n_binary* rval;}                           n_neg; // done.
typedef struct{node_type ntype;char* name;vec* args;}                     n_call; // done.
typedef struct{node_type ntype;vec* body;}                                n_block; // done.
typedef struct{node_type ntype;char* name,*type;vec* args;n_block* body;} n_func; // done.
typedef struct{node_type ntype;char* name,*type;}                         n_arg; // done.
typedef struct{node_type ntype;char* name;n_binary* rval;}                n_assign; // done.
typedef struct{node_type ntype;n_binary* rval;}                           n_return; // done.
typedef struct{node_type ntype;char* name,*type;}                         n_decl; // done.
typedef struct{node_type ntype;n_binary* cond;n_block* body;}             n_ifs; // done.
typedef struct{node_type ntype;n_binary* cond;n_block* t,*f;}             n_ife;
typedef struct{node_type ntype;n_binary* cond;n_block* body;}             n_while;
typedef struct{node_type ntype;n_binary* rval;}                           n_print;
typedef struct{node_type ntype;char* name;}                               n_input;

void iprintf(int i,const char* fmt,...){
 for(int j=0;j<i;i++){printf(" ");}
 va_list args;
 va_start(args,fmt);
 vprintf(fmt,args);
 va_end(args);
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

// assignment | return | declaration | if | while | print | input

n_binary* parse_binary(vec*,int*);

n_binary* parse_parens(vec* tokens,int* p){
 (*p)++;
 n_binary* b=parse_binary(tokens,p);
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
 lexpectm("(",tokens,p);
 c->args=newvec();
 if(!lmatch(")",tokens,p)){
  while(1){
   vecadd(c->args,parse_binary(tokens,p));
   if(lmatch(")",tokens,p)){break;}
   lexpectm(",",tokens,p);
  }
 }
 (*p)++;
 texpectm("newline",tokens,p);
 return c;
}
n_node* parse_term(vec* tokens,int* p){
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

n_binary* parse_comp(vec* tokens,int* p){
 n_binary* b=malloc(sizeof(n_binary));
 b->values=newvec();
 b->ops=newvec();
 vecadd(b->values,parse_term(tokens,p));
 while(lmatch("*",tokens,p)||lmatch("/",tokens,p)){
  char* op=malloc(strlen(fetch(tokens,p)->lexeme)+1);
  vecadd(b->ops,strcat(strcpy(op,fetch(tokens,p)->lexeme),"\0"));
  (*p)++;
  vecadd(b->values,parse_term(tokens,p));
 }
 return b;
}

n_binary* parse_binary(vec* tokens,int* p){
 n_binary* b=malloc(sizeof(n_binary));
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
 return b;
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

#define USAGE(name,cond) if (cond) { fprintf(stderr,"\x1b[32mUsage: %s <filename>\x1b[0m\n",name); exit(-1); }
int main(int argc, char** argv)
{
 USAGE(argv[0],argc < 2);
 int debug = 0;
 FILE* fp = fopen(argv[1],"r");
 USAGE(argv[0],fp == NULL);
 fseek(fp,0,SEEK_END);
 long len = ftell(fp);
 fseek(fp,0,SEEK_SET);
 char* buf = malloc(len+1);
 fread(buf,1,len,fp);
 fclose(fp);
 buf[len] = 0;
 vec* tokens = tokenize(buf);
#if __DEBUG__
  for (int i=0;i<tokens->len;i++)
  {
   token* t = (token*)(vecget(tokens,i));
   printf("Token: `%s`,`%s`.\n",t->type,t->lexeme);
  }
#endif
 n_prog* n = parse(tokens);
 for (int i=0;i<tokens->len;i++) { tokfree((token*)vecget(tokens,i)); }
 vecfree(tokens);
 free(buf);
 return 0;
}
