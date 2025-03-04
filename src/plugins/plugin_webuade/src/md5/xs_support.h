#ifndef XS_SUPPORT_H
#define XS_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "types.h"

#include <stdio.h>
/*

*/
#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(x) /* stub */
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif


/* Standard gettext macros
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define _LIBINTL_H
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define xs_file_t FILE
#define xs_fopen(a,b) fopen(a,b)
#define xs_fclose(a) fclose(a)
#define xs_fgetc(a) fgetc(a)
#define xs_fread(a,b,c,d) fread(a,b,c,d)
#define xs_feof(a) feof(a)
#define xs_ferror(a) ferror(a)
#define xs_ftell(a) ftell(a)
#define xs_fseek(a,b,c) fseek(a,b,c)

guint16 xs_fread_be16(xs_file_t *);
guint32 xs_fread_be32(xs_file_t *);
gint    xs_fload_buffer(const gchar *, guint8 **, size_t *);


/* Misc functions
 */
char    *xs_strncpy(char *, const char *, size_t);
int    xs_pstrcpy(char **, const char *);
int    xs_pstrcat(char **, const char *);
void    xs_pnstrcat(char *, size_t, const char *);
char    *xs_strrchr(char *, const char);
void    xs_findnext(const char *, size_t *);
void    xs_findeol(const char *, size_t *);
void    xs_findnum(const char *, size_t *);

#ifdef __cplusplus
}
#endif
#endif /* XS_SUPPORT_H */
