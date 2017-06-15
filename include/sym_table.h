#ifndef _SYM_TABLE_H
#define _SYM_TABLE_H
#include "vec.h"
#include "sym_table_node.h"
typedef struct{
 vec* keys;
 vec* vals;
}sym_table;

sym_table* newsymtable(void);
void symtabledecl(sym_table*,char*,enum stntype);
sym_table_node* symtableget(sym_table* s,char*);

#endif /* end of include guard: _SYM_TABLE_H */
