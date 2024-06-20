#include "xs_support.h"
#include <ctype.h>


guint16 xs_fread_be16(xs_file_t *f)
{
    return (((guint16) xs_fgetc(f)) << 8) | ((guint16) xs_fgetc(f));
}


guint32 xs_fread_be32(xs_file_t *f)
{
    return (((guint32) xs_fgetc(f)) << 24) |
        (((guint32) xs_fgetc(f)) << 16) |
        (((guint32) xs_fgetc(f)) << 8) |
        ((guint32) xs_fgetc(f));
}

void xs_findnext(const gchar *str, size_t *pos)
{
    while (str[*pos] && isspace(str[*pos]))
        (*pos)++;
}


void xs_findeol(const gchar *str, size_t *pos)
{
    while (str[*pos] && (str[*pos] != '\n') && (str[*pos] != '\r'))
        (*pos)++;
}


void xs_findnum(const gchar *str, size_t *pos)
{
    while (str[*pos] && isdigit(str[*pos]))
        (*pos)++;
}

