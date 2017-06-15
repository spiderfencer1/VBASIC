#include <stdlib.h>
#include <string.h>

#include "../include/sym_table_node.h"

sym_table_node* newsymtablenode(int offset,enum stntype type,char* name){
 sym_table_node* stn = malloc(sizeof(sym_table_node*));
 stn->offset = offset;
 stn->type = type;
 strcpy((stn->name = malloc(strlen(name)+1)),name);
 return stn;
}
