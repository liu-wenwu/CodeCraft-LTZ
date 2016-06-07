#define HAVE_GETRUSAGE

/* #undef HAVE_GETRUSAGE */
#undef HAVE_GETRUSAGE

double GetTime(void);


#ifdef HAVE_GETRUSAGE
#include <sys/time.h>
#include <sys/resource.h>

double GetTime()
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
}

#else

#include <time.h>

double GetTime()
{
    return (double) clock() / CLOCKS_PER_SEC;
}

#endif
