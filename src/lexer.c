#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"

token* newtok(char* type, char* lexeme)
{
 token* t = malloc(sizeof(token));
 strcat(strcpy((t->type = malloc(strlen(type)+1)),type),"\0");
 strcat(strcpy((t->lexeme = malloc(strlen(lexeme)+1)),lexeme),"\0");
 return t;
}
void tokfree(token* t) { free(t->type); free(t->lexeme); free(t); }

vec* tokenize(char* buf)
{
 vec* tokens = newvec();
 int pos = 0;

 char* keywords[] = 
 {
  "Function","End","Integer","As","Dim","Let","Return",
  "If","Then","Else","Do","While","Loop","Print","Input"
 };

 while (pos < strlen(buf))
 {
  char look = buf[pos++];

  if (look == '\n')
  {
   int count = 1;
   for(;pos<strlen(buf)&&buf[pos]=='\n';count++,pos++);
   char* nl=malloc(count+1);
   for(int i=0;i<count;i++){nl[i]='n';}
   nl[count]=0;
   vecadd(tokens,newtok("newline",nl));
  }

  int flg = 0;
  for (int i=0;i<sizeof(keywords)/sizeof(char*);i++)
  {
   if (strncmp(&buf[pos-1],keywords[i],strlen(keywords[i])) == 0)
   {
    pos += strlen(keywords[i]) - 1;
    vecadd(tokens,newtok(keywords[i],keywords[i]));
    flg = 1;
    break;
   }
  }
  if (flg) { continue; }
  else if (DIGIT(look))
  {
   char* num = malloc(1);
   num[0] = look;
   int i = 1;
   while (pos < strlen(buf) && DIGIT(buf[pos]))
   {
    num = realloc(num,i+1);
    num[i++] = buf[pos++];
   }
   num = realloc(num,i+1);
   num[i] = 0;
   vecadd(tokens,newtok("num",num));
   free(num);
  }
  else if (IDENT(look))
  {
   char* var = malloc(1);
   var[0] = look;
   int i = 1;
   while (pos < strlen(buf) && (DIGIT(buf[pos]) || IDENT(buf[pos])))
   {
    var = realloc(var,i+1);
    var[i++] = buf[pos++];
   }
   var = realloc(var,i+1);
   var[i] = 0;
   vecadd(tokens,newtok("var",var));
   free(var);
  }
  else
  {
   switch (look)
   {
    case '+':
     vecadd(tokens,newtok("plus","+"));
     break;
    case '-':
     vecadd(tokens,newtok("minus","-"));
     break;
    case '*':
     vecadd(tokens,newtok("mult","*"));
     break;
    case '/':
     vecadd(tokens,newtok("div","/"));
     break;
    case '(':
     vecadd(tokens,newtok("lparen","("));
     break;
    case ')':
     vecadd(tokens,newtok("rparen",")"));
     break;
    case '<':
     if (pos < strlen(buf) && buf[pos] == '=') { pos++; vecadd(tokens,newtok("lte","<=")); break; }
     vecadd(tokens,newtok("ltn","<"));
     break;
    case '>':
     if (pos < strlen(buf) && buf[pos] == '=') { pos++; vecadd(tokens,newtok("gte",">=")); break; }
     vecadd(tokens,newtok("gtn",">"));
     break;
    case '=':
					if (pos < strlen(buf) && buf[pos] == '='){
						pos++;
						vecadd(tokens,newtok("equal","=="));
					}else{
						vecadd(tokens,newtok("assign","="));
					}
     break;
    case ',':
     vecadd(tokens,newtok("comma",","));
     break;
    default:
     break;
   }
  }
 }
 token* t = (token*)(vecget(tokens,tokens->len-1));
 if (strcmp(t->type,"newline") != 0) { vecadd(tokens,newtok("newline","1")); }
 return tokens;
}
