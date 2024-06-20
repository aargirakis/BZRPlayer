#ifndef XS_LENGTH_H
#define XS_LENGTH_H



#ifdef __cplusplus
extern "C" {
#endif

//added by blazer
#include <stddef.h>
#include "xs_md5.h"
/* Types
 */
typedef struct _sldb_node_t {
    xs_md5hash_t    md5Hash;    /* 128-bit MD5 hash-digest */
    gint            nlengths;   /* Number of lengths */
    gint            *lengths;   /* Lengths in seconds */
    struct _sldb_node_t *prev, *next;
} sldb_node_t;


typedef struct {
    sldb_node_t     *nodes,
                    **pindex;
    size_t          n;
} xs_sldb_t;


/* Functions
 */
gint            xs_sldb_read(xs_sldb_t *, const gchar *);
gint            xs_sldb_index(xs_sldb_t *);
void            xs_sldb_free(xs_sldb_t *);
sldb_node_t *   xs_sldb_get(xs_sldb_t *, const gchar *);



gint xs_get_sid_hash(const gchar *filename, xs_md5hash_t hash);
gint xs_get_sid_hash_old(const gchar *filename, xs_md5hash_t hash);


#ifdef __cplusplus
}
#endif
#endif /* XS_LENGTH_H */
