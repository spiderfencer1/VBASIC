#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../include/error.h"

void error(const char* fmt, ...)
{
 printf("\x1b[31m");
 va_list args;
 va_start(args,fmt);
 vprintf(fmt,args);
 va_end(args);
 printf("\x1b[0m\n");
 exit(-1);
}
