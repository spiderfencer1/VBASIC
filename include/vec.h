#ifndef __VEC_H
#define __VEC_H

typedef struct
{
 int len;
 void** data;
} vec;
vec* newvec(void);
void vecfree(vec*);
void* vecadd(vec*,void*);
void* vecget(vec*,int);
void* vecset(vec*,int,void*);
vec* initvec(int,...);

#endif
