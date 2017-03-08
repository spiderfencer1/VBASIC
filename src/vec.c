#include <stdarg.h>
#include <stdlib.h>

#include "../include/vec.h"

vec* newvec(void)
{
 vec* v = malloc(sizeof(vec));
 v->len = 0;
 v->data = malloc(0);
 return v;
}
void vecfree(vec* v) { free(v->data); free(v); }
void* vecadd(vec* v, void* e)
{
 v->data = realloc(v->data, sizeof(void*) * v->len + sizeof(e));
 return v->data[v->len++] = e;
}
void* vecget(vec* v, int i) { return (0 <= i && i < v->len) ? v->data[i] : NULL; }
void* vecset(vec* v, int i, void* e) { return (0 <= i && i < v->len) ? v->data[i] = e : NULL; }
vec* initvec(int nargs,...)
{
 vec* v = newvec();
 va_list args;
 va_start(args,nargs);
 for (int i=0;i<nargs;i++) { vecadd(v,va_arg(args,void*)); }
 return v;
}
