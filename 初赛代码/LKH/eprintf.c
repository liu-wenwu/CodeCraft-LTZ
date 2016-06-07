#include "LKH.h"
#include <stdarg.h>


void eprintf(const char *fmt, ...)
{
    va_list args;

    if (LastLine && *LastLine)
        fprintf(stderr, "\n%s\n", LastLine);
    fprintf(stderr, "\n*** Error ***\n");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
   
}
