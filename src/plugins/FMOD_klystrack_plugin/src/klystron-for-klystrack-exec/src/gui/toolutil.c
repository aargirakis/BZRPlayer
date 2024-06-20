/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "toolutil.h"
#include "msgbox.h"
#include "filebox.h"
#include <string.h>
#include "macros.h"

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#endif


char * open_dialog_fn(const char *mode, const char *title, const char *filter, GfxDomain *domain, GfxSurface *gfx, const Font *largefont, const Font *smallfont, const char *deffilename, char *filename, int filename_size)
{
	if (deffilename)
		strncpy(filename, deffilename, filename_size - 1);
	else
		strcpy(filename, "");
	
	if (filebox(title, mode[0] == 'w' ? FB_SAVE : FB_OPEN, filename, filename_size - 1, filter, domain, gfx, largefont, smallfont) == FB_OK)
	{
		return filename;
	}
	else
		return NULL;
}


FILE *open_dialog(const char *mode, const char *title, const char *filter, GfxDomain *domain, GfxSurface *gfx, const Font *largefont, const Font *smallfont, const char *deffilename)
{
	char filename[5000];
	
	if (open_dialog_fn(mode, title, filter, domain, gfx, largefont, smallfont, deffilename, filename, sizeof(filename)))
	{
		FILE * f = fopen(filename, mode);
		if (!f) msgbox(domain, gfx, largefont, "Could not open file", MB_OK);
		return f;
	}
	else
		return NULL;
}


int confirm(GfxDomain *domain, GfxSurface *gfx, const Font *font, const char *msg)
{
	return msgbox(domain, gfx, font, msg, MB_YES|MB_NO) == MB_YES; // MessageBox(0, msg, "Confirm", MB_YESNO) == IDYES;
}

int confirm_ync(GfxDomain *domain, GfxSurface *gfx, const Font *font, const char *msg)
{
	int r = msgbox(domain, gfx, font, msg, MB_YES|MB_NO|MB_CANCEL);
	
	if (r == MB_YES)
	{
		return 1;
	}
	if (r == MB_NO)
	{
		return -1;
	}
	
	return 0;
	
}


char * expand_tilde(const char * path)
{
	if (path[0] != '~') return NULL;
	
#ifndef WIN32	
	const char *rest = strchr(path, '/');
#else
	const char *rest = strchr(path, '/');
	if (!rest) rest = strchr(path, '\\');
#endif	
	
	size_t rest_len = 0;
	
	if (rest != NULL)
	{
		rest_len = strlen(rest);
	}
	
	const char *homedir = NULL;
	
#ifndef WIN32		
	struct passwd *pw = getpwuid(getuid());

	if (pw)
		homedir = pw->pw_dir;
#else
	homedir = getenv("USERPROFILE");
#endif
	
	char * final = malloc(strlen(homedir) + rest_len + 2);
	
	strcpy(final, homedir);
	
	if (rest) strcat(final, rest);
	
	return final;
}
