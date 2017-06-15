#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../include/vec.h"
#include "../include/error.h"
#include "../include/parser.h"
#include "../include/generator.h"

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
