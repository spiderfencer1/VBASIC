#include "../include/sym_table.h"
#include "../include/vec.h"

sym_table* newsymtable(void){
 sym_table* st = malloc(sizeof(sym_table));
 st->keys = newvec();
 st->vals = newvec();
 return st;
}

void symtableset(char* key,int val){
}
