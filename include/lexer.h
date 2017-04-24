#ifndef _LEXER_H
#define _LEXER_H

#include "vec.h"

#define DIGIT(chr) ('0' <= chr && chr <= '9')
#define IDENT(chr) (('A' <= chr && chr <= 'Z') || ('a' <= chr && chr <= 'z') || chr == '_')

typedef struct
{
 char* type, *lexeme;
} token;
token* newtok(char*,char*);
void tokfree(token*);

vec* tokenize(char*);

#endif
