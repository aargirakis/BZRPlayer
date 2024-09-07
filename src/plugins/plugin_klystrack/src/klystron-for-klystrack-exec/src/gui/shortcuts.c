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


#include "shortcuts.h"
#include <string.h>
#include <ctype.h>

void do_shortcuts(SDL_KeyboardEvent *e, const KeyShortcut *shortcuts)
{
	if (e->type != SDL_KEYDOWN)
		return;

	for (int i = 0 ; shortcuts[i].action ; ++i)
	{
		if (e->keysym.sym == shortcuts[i].key
			&& (!(e->keysym.mod & KMOD_SHIFT) == !(shortcuts[i].mod & KMOD_SHIFT))
			&& (!(e->keysym.mod & KMOD_CTRL) == !(shortcuts[i].mod & KMOD_CTRL))
			&& (!(e->keysym.mod & KMOD_ALT) == !(shortcuts[i].mod & KMOD_ALT))
		)
		{
			shortcuts[i].action((void*)shortcuts[i].p1, (void*)shortcuts[i].p2, (void*)shortcuts[i].p3);
			e->keysym.sym = 0;
			break;
		}
	}
}


static const char *upcase(char *str)
{
	for (char *c = str ; *c ; ++c)
		*c = toupper(*c);

	return str;
}


const char * get_shortcut_string(const KeyShortcut *sc)
{
	static char buffer[100] = { 0 };
	strcpy(buffer, "");

	if (sc->mod & KMOD_CTRL)
		strncat(buffer, "ctrl-", sizeof(buffer) - 1);

	if (sc->mod & KMOD_ALT)
		strncat(buffer, "alt-", sizeof(buffer) - 1);

	if (sc->mod & KMOD_SHIFT)
		strncat(buffer, "shift-", sizeof(buffer) - 1);

	char keyname[50] = { 0 };

	if (sc->key >= SDLK_KP_DIVIDE && sc->key <= SDLK_KP_EQUALSAS400)
	{
		strncpy(keyname, SDL_GetKeyName(sc->key), sizeof(keyname) - 1);
		keyname[2] = ' ';
		keyname[3] = keyname[7];
		keyname[4] = '\0';
		keyname[0] = 'K';
		keyname[1] = 'P';
	}
	else
	{
		strncpy(keyname, SDL_GetKeyName(sc->key), 3);
	}

	strncat(buffer, keyname, sizeof(buffer) - 1);
	return upcase(buffer);
}
