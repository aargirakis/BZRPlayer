/*  
   XMMS-SID - SIDPlay input plugin for X MultiMedia System (XMMS)

   Get song length from SLDB for PSID/RSID files
   
   Programmed and designed by Matti 'ccr' Hamalainen <ccr@tnsp.org>
   (C) Copyright 1999-2007 Tecnic Software productions (TNSP)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "xs_length.h"
#include "xs_support.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Size of data buffer used for SID-tune MD5 hash calculation.
 * If this is too small, the computed hash will be incorrect.
 * Largest SID files I've seen are ~70kB. */

//this was originally in xmms-sid.h /blazer
#define XS_SIDBUF_SIZE        (80*1024)


/* Compute md5hash of given SID-file
 */
typedef struct {
    gchar magicID[4];    /* "PSID" / "RSID" magic identifier */
    guint16 version,    /* Version number */
        dataOffset,    /* Start of actual c64 data in file */
        loadAddress,    /* Loading address */
        initAddress,    /* Initialization address */
        playAddress,    /* Play one frame */
        nSongs,        /* Number of subsongs */
        startSong;    /* Default starting song */
    guint32 speed;        /* Speed */
    gchar sidName[32];    /* Descriptive text-fields, ASCIIZ */
    gchar sidAuthor[32];
    gchar sidCopyright[32];
} psidv1_header_t;


typedef struct {
    guint16 flags;        /* Flags */
    guint8 startPage, pageLength;
    guint16 reserved;
} psidv2_header_t;




gint xs_get_sid_hash(const gchar *filename, xs_md5hash_t hash)
{
    xs_file_t *inFile;
    xs_md5state_t inState;
    psidv1_header_t psidH;
    psidv2_header_t psidH2;
    guint8 *songData;
    guint8 ib8[2], i8;
    gint index, result;

    /* Try to open the file */
    if ((inFile = xs_fopen(filename, "rb")) == NULL)
        return -1;

    /* Allocate buffer */
    songData = (guint8 *) malloc(XS_SIDBUF_SIZE * sizeof(guint8));
    if (!songData) {
        xs_fclose(inFile);
        //xs_error("Error allocating temp data buffer for file '%s'\n", filename);
        return -3;
    }

    /* Read data to buffer */
    result = xs_fread(songData, sizeof(guint8), XS_SIDBUF_SIZE, inFile);
    xs_fclose(inFile);

    /* Initialize and start MD5-hash calculation */
    xs_md5_init(&inState);


    xs_md5_append(&inState, songData, result);


    /* Free buffer */
    free(songData);


    /* Calculate the hash */
    xs_md5_finish(&inState, hash);

    return 0;
}

//this function was static, but i cant compile with that.... /blazer
gint xs_get_sid_hash_old(const gchar *filename, xs_md5hash_t hash)
{
    xs_file_t *inFile;
    xs_md5state_t inState;
    psidv1_header_t psidH;
    psidv2_header_t psidH2;
    guint8 *songData;
    guint8 ib8[2], i8;
    gint index, result;

    /* Try to open the file */
    if ((inFile = xs_fopen(filename, "rb")) == NULL)
        return -1;

    /* Read PSID header in */
    xs_fread(psidH.magicID, sizeof(psidH.magicID), 1, inFile);
    if (strncmp(psidH.magicID, "PSID", 4) && strncmp(psidH.magicID, "RSID", 4)) {
        xs_fclose(inFile);
        //xs_error("Not a PSID or RSID file '%s'\n", filename);
        return -2;
    }

    psidH.version = xs_fread_be16(inFile);
    psidH.dataOffset = xs_fread_be16(inFile);
    psidH.loadAddress = xs_fread_be16(inFile);
    psidH.initAddress = xs_fread_be16(inFile);
    psidH.playAddress = xs_fread_be16(inFile);
    psidH.nSongs = xs_fread_be16(inFile);
    psidH.startSong = xs_fread_be16(inFile);
    psidH.speed = xs_fread_be32(inFile);

    xs_fread(psidH.sidName, sizeof(gchar), sizeof(psidH.sidName), inFile);
    xs_fread(psidH.sidAuthor, sizeof(gchar), sizeof(psidH.sidAuthor), inFile);
    xs_fread(psidH.sidCopyright, sizeof(gchar), sizeof(psidH.sidCopyright), inFile);
    
    if (xs_feof(inFile) || xs_ferror(inFile)) {
        xs_fclose(inFile);
        //xs_error("Error reading SID file header from '%s'\n", filename);
        return -4;
    }
    
    /* Check if we need to load PSIDv2NG header ... */
    psidH2.flags = 0;    /* Just silence a stupid gcc warning */
    
    if (psidH.version == 2) {
        /* Yes, we need to */
        psidH2.flags = xs_fread_be16(inFile);
        psidH2.startPage = xs_fgetc(inFile);
        psidH2.pageLength = xs_fgetc(inFile);
        psidH2.reserved = xs_fread_be16(inFile);
    }

    /* Allocate buffer */
    songData = (guint8 *) malloc(XS_SIDBUF_SIZE * sizeof(guint8));
    if (!songData) {
        xs_fclose(inFile);
        //xs_error("Error allocating temp data buffer for file '%s'\n", filename);
        return -3;
    }

    /* Read data to buffer */
    result = xs_fread(songData, sizeof(guint8), XS_SIDBUF_SIZE, inFile);
    xs_fclose(inFile);

    /* Initialize and start MD5-hash calculation */
    xs_md5_init(&inState);

    if (psidH.loadAddress == 0) {
        /* Strip load address (2 first bytes) */
        xs_md5_append(&inState, &songData[2], result - 2);
    } else {
        /* Append "as is" */
        xs_md5_append(&inState, songData, result);
    }

    /* Free buffer */
    free(songData);

    /* Append header data to hash */
#define XSADDHASH(QDATAB) do {                    \
    ib8[0] = (QDATAB & 0xff);                \
    ib8[1] = (QDATAB >> 8);                    \
    xs_md5_append(&inState, (guint8 *) &ib8, sizeof(ib8));    \
    } while (0)

    XSADDHASH(psidH.initAddress);
    XSADDHASH(psidH.playAddress);
    XSADDHASH(psidH.nSongs);
#undef XSADDHASH

    /* Append song speed data to hash */
    i8 = 0;
    for (index = 0; (index < psidH.nSongs) && (index < 32); index++) {
        i8 = (psidH.speed & (1 << index)) ? 60 : 0;
        xs_md5_append(&inState, &i8, sizeof(i8));
    }

    /* Rest of songs (more than 32) */
    for (index = 32; index < psidH.nSongs; index++) {
        xs_md5_append(&inState, &i8, sizeof(i8));
    }

    /* PSIDv2NG specific */
    if (psidH.version == 2) {
        /* SEE SIDPLAY HEADERS FOR INFO */
        i8 = (psidH2.flags >> 2) & 3;
        if (i8 == 2)
            xs_md5_append(&inState, &i8, sizeof(i8));
    }

    /* Calculate the hash */
    xs_md5_finish(&inState, hash);

    return 0;
}



