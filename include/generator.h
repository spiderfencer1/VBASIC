#ifndef _GENERATOR_H
#define _GENERATOR_H

#include "vec.h"
#include "parser.h"

typedef struct{
 char* name,*type;
 int idx;
}fnvar;

typedef struct{
 char* name;
 vec* args; // vec<fnvar*> 
 vec* vars; // vec<fnvar*>
}fntempl;

void generate_let(n_assign*);
void generate_return(n_return*);
void generate_if(n_node*);
void generate_while(n_while*);
void generate_print(n_print*);
void generate_input(n_input*);
void generate_block(n_block*);
void generate_funct(fntempl*,n_func*);
void generate(n_prog*,vec*);

#endif
