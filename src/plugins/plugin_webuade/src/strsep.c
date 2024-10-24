#include <string.h>

char* strsep(char** stringp, const char* delim)
{
    char* rv = *stringp;
    if (rv)
    {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0;
    }

    return rv;
}
