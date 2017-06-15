#ifndef _SYM_TABLE_NODE_H
#define _SYM_TABLE_NODE_H

typedef struct{
 int offset;
 enum stntype {NUM_LOCAL,NUM_PARAM} type;
 char* name;
}sym_table_node;
sym_table_node* newsymtablenode(int,enum stntype,char*);
#endif /* end of include guard: _SYM_TABLE_NODE_H */
