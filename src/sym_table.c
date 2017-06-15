#include "../include/error.h"
#include "../include/sym_table.h"
#include "../include/sym_table_node.h"
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

void symtabledecl(sym_table* s,char* k,enum stntype type)
{
 int nodes_of_type = 0;
 for(int i=0;i<s->keys->len;i++)
 {
  if(strcmp(((char*)vecget(s->keys,i)),k) == 0)
  {
   error("Duplicate variable declaration: %s.",k);
   return;
  }
  sym_table_node* n = (sym_table_node*)vecget(s->vals,i); 
  if(n->type == type)
  {
   nodes_of_type++; 
  }
 }
 int offset;
 if(type == NUM_LOCAL){
  offset = -nodes_of_type * 4 - 4;
 }else if(type == NUM_PARAM){
  offset = nodes_of_type * 4 + 8;
 }
 sym_table_node* stn = newsymtablenode(offset,type,k);
 vecadd(s->keys,k);
 vecadd(s->vals,stn);
}

sym_table_node* symtableget(sym_table* s,char* k)
{
 for(int i=0;i<s->keys->len;i++)
 {
  if(strcmp(((char*)vecget(s->keys,i)),k) == 0)
  {
   return (sym_table_node*)vecget(s->vals,i);
  }
 }
 return NULL; 
}
