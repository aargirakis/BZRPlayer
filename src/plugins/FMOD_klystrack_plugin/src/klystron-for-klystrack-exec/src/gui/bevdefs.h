#ifndef BEVDEFS_H
#define BEVDEFS_H

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

#define BEV_SIZE 16

enum
{
	BEV_SLIDER_BG,
	BEV_SLIDER_HANDLE,
	BEV_SLIDER_HANDLE_ACTIVE,
	BEV_MENUBAR,
	BEV_MENU,
	BEV_MENU_SELECTED,
	BEV_BUTTON,
	BEV_BUTTON_ACTIVE,
	BEV_FIELD,
	BEV_SEPARATOR,
	BEV_SELECTED_ROW,
	BEV_CURSOR,
	BEV_USER
};


enum
{
	DECAL_UPARROW,
	DECAL_DOWNARROW,
	DECAL_GRAB_VERT,
	DECAL_GRAB_HORIZ,
	DECAL_TICK,
	DECAL_PLUS,
	DECAL_MINUS,
	DECAL_RIGHTARROW,
	DECAL_LEFTARROW,
	DECAL_CLOSE,
	DECAL_FAVORITE,
	DECAL_UNFAVORITE,
	DECAL_USER,
};

#endif
