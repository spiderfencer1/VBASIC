#ifndef __PARSER_H
#define __PARSER_H

#include "error.h"
#include "lexer.h"
#include "vec.h"

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

void iprintf(int,const char*,...);
void print_node(n_node*,int);
token* fetch(vec*,int*);
int tmatch(char*,vec*,int*);
int lmatch(char*,vec*,int*);
void texpect(char*,vec*,int*);
void lexpect(char*,vec*,int*);
void texpectm(char*,vec*,int*);
void lexpectm(char*,vec*,int*);
n_node* parse_parens(vec*,int*);
n_const* parse_const(vec*,int*);
n_var* parse_var(vec*,int*);
n_neg* parse_neg(vec*,int*);
n_call* parse_call(vec*,int*);
n_node* parse_factor(vec*,int*);
n_node* parse_term(vec*,int*);
n_node* parse_comp(vec*,int*);
n_node* parse_binary(vec*,int*);
n_assign* parse_let(vec*,int*);
n_return* parse_return(vec*,int*);
n_decl* parse_decl(vec*,int*);
n_ife* parse_ife(n_ifs*,vec*,int*);
n_node* parse_if(vec*,int*);
n_while* parse_while(vec*,int*);
n_print* parse_print(vec*,int*);
n_input* parse_input(vec*,int*);
n_block* parse_block(vec*,int*);
n_arg* parse_arg(vec*,int*);
n_func* parse_func(vec*,int*);
n_prog* parse(vec*);

#endif
