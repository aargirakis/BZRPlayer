/* Extended Module Player
 * Copyright (C) 1996-2013 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include <ctype.h>
#include <sys/types.h>
#include <stdarg.h>
#ifndef WIN32
#include <dirent.h>
#endif

#include "xmp.h"
#include "common.h"
#include "period.h"
#include "loader.h"


char *copy_adjust(char *s, uint8 *r, int n)
{
	int i;

	memset(s, 0, n + 1);
	strncpy(s, (char *)r, n);

	for (i = 0; s[i] && i < n; i++) {
		if (!isprint((int)s[i]) || ((uint8)s[i] > 127))
			s[i] = '.';
	}

	while (*s && (s[strlen(s) - 1] == ' '))
		s[strlen(s) - 1] = 0;

	return s;
}

int test_name(uint8 *s, int n)
{
	int i;

	/* ACS_Team2.mod has a backspace in instrument name */
	for (i = 0; i < n; i++) {
		if (s[i] > 0x7f)
			return -1;
		if (s[i] > 0 && s[i] < 32 && s[i] != 0x08)
			return -1;
	}

	return 0;
}

void read_title(FILE *f, char *t, int s)
{
	uint8 buf[XMP_NAME_SIZE];

	if (t == NULL)
		return;

	if (s >= XMP_NAME_SIZE)
		s = XMP_NAME_SIZE -1;

	memset(t, 0, s + 1);

	fread(buf, 1, s, f);
	buf[s] = 0;
	copy_adjust(t, buf, s);
}

void cvt_pt_event(struct xmp_event *event, uint8 *mod_event)
{
	event->note = period_to_note((LSN(mod_event[0]) << 8) + mod_event[1]);
	event->ins = ((MSN(mod_event[0]) << 4) | MSN(mod_event[2]));
	event->fxt = LSN(mod_event[2]);
	event->fxp = mod_event[3];

	disable_continue_fx(event);
}

void disable_continue_fx(struct xmp_event *event)
{
	if (!event->fxp) {
		switch (event->fxt) {
		case 0x05:
			event->fxt = 0x03;
			break;
		case 0x06:
			event->fxt = 0x04;
			break;
		case 0x01:
		case 0x02:
		case 0x0a:
			event->fxt = 0x00;
		}
	}
}

#ifndef WIN32

/* Given a directory, see if file exists there, ignoring case */

int check_filename_case(char *dir, char *name, char *new_name, int size)
{
	int found = 0;
	DIR *dirfd;
	struct dirent *d;

	dirfd = opendir(dir);
	if (dirfd == NULL)
		return 0;
 
	while ((d = readdir(dirfd))) {
		if (!strcasecmp(d->d_name, name)) {
			found = 1;
			break;
		}
	}

	if (found)
		strncpy(new_name, d->d_name, size);

	closedir(dirfd);

	return found;
}

#else

/* FIXME: implement functionality for Win32 */

int check_filename_case(char *dir, char *name, char *new_name, int size)
{
	return 0;
}

#endif

void get_instrument_path(struct module_data *m, char *path, int size)
{
	if (m->instrument_path) {
		strncpy(path, m->instrument_path, size);
	} else if (getenv("XMP_INSTRUMENT_PATH")) {
		strncpy(path, getenv("XMP_INSTRUMENT_PATH"), size);
	} else {
		strncpy(path, ".", size);
	}
}

void set_type(struct module_data *m, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	vsnprintf(m->mod.type, XMP_NAME_SIZE, fmt, ap);
	va_end(ap);
}
