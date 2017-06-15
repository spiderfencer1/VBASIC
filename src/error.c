#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../include/error.h"

void error(const char* fmt, ...)
{
 fprintf(stderr,"\x1b[31m");
 va_list args;
 va_start(args,fmt);
 vfprintf(stderr,fmt,args);
 va_end(args);
 fprintf(stderr,"\x1b[0m\n");
 exit(-1);
}
