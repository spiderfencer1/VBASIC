#include "../include/sym_table.h"
#include "../include/vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sym_table* newsymtable(void)
{
 sym_table* st = malloc(sizeof(sym_table));
 st->keys = newvec();
 st->vals = newvec();
 return st;
}

void symtableset(sym_table* s,char* k,void* v)
{
 for(int i=0;i<s->keys->len;i++)
 {
  if(strcmp(((char*)vecget(s->keys,i)),k) == 0)
  {
   vecset(s->vals,i,v);
  }
 }
 vecadd(s->keys,k);
 vecadd(s->vals,v);
}

void* symtableget(sym_table* s,char* k)
{
 for(int i=0;i<s->keys->len;i++)
 {
  if(strcmp(((char*)vecget(s->keys,i)),k) == 0)
  {
   return vecget(s->vals,i);
  }
 }
 return NULL; 
}
