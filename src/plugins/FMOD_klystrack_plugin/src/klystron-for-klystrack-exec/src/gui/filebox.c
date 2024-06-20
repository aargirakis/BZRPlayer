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

#include "filebox.h"
#include "msgbox.h"
#include "gui/bevel.h"
#include "gui/bevdefs.h"
#include "dialog.h"
#include "gfx/gfx.h"
#include "gui/view.h"
#include "gui/mouse.h"
#include "toolutil.h"
#include "slider.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef WIN32

#include <windows.h>

#endif

#define SCROLLBAR 10
#define TOP_LEFT 0
#define TOP_RIGHT 0
#define MARGIN 8
#define SCREENMARGIN 32
#define TITLE 14
#define FIELD 14
#define CLOSE_BUTTON 12
#define PATH 10
#define ELEMWIDTH data.elemwidth
#define LIST_WIDTH data.list_width
#define BUTTONS 16

enum { FB_DIRECTORY, FB_FILE };

enum { FOCUS_LIST, FOCUS_FIELD };

static GfxDomain *domain;

typedef struct
{
	int type;
	char *name;
	char *display_name;
} File;

static struct
{
	int mode;
	const char *title;
	File *files;
	int n_files;
	SliderParam scrollbar;
	int list_position, selected_file;
	File * picked_file;
	int focus;
	char field[256];
	int editpos;
	int quit;
	char path[1024], fullpath[2048];
	const Font *largefont, *smallfont;
	GfxSurface *gfx;
	int elemwidth, list_width;
	bool selected;
} data;

static char **favorites = NULL;
static int n_favorites = 0;
static char last_picked_file[250] = {0};

static void file_list_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void title_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void field_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void path_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void window_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void buttons_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);

static const View filebox_view[] =
{
	{{ SCREENMARGIN, SCREENMARGIN, -SCREENMARGIN, -SCREENMARGIN }, window_view, &data, -1},
	{{ MARGIN+SCREENMARGIN, SCREENMARGIN+MARGIN, -MARGIN-SCREENMARGIN, TITLE - 2 }, title_view, &data, -1},
	{{ MARGIN+SCREENMARGIN, SCREENMARGIN+MARGIN + TITLE, -MARGIN-SCREENMARGIN, PATH - 2 }, path_view, &data, -1},
	{{ MARGIN+SCREENMARGIN, SCREENMARGIN+MARGIN + TITLE + PATH, -MARGIN-SCREENMARGIN, FIELD - 2 }, field_view, &data, -1},
	{{ -SCROLLBAR-MARGIN-SCREENMARGIN, SCREENMARGIN+MARGIN + TITLE + PATH + FIELD, SCROLLBAR, -MARGIN-SCREENMARGIN-BUTTONS }, slider, &data.scrollbar, -1},
	{{ SCREENMARGIN+MARGIN, SCREENMARGIN+MARGIN + TITLE + PATH + FIELD, -SCROLLBAR-MARGIN-1-SCREENMARGIN, -MARGIN-SCREENMARGIN-BUTTONS }, file_list_view, &data, -1},
	{{ SCREENMARGIN+MARGIN, -SCREENMARGIN-MARGIN-BUTTONS+2, -MARGIN-SCREENMARGIN, BUTTONS-2 }, buttons_view, &data, -1},
	{{0, 0, 0, 0}, NULL}
};


static void setfocus(int focus)
{
	data.focus = focus;

	if (focus == FOCUS_FIELD)
	{
		SDL_StartTextInput();
	}
	else
		SDL_StopTextInput();
}


static void add_file(int type, const char *name)
{
	const int block_size = 256;

	if ((data.n_files & (block_size - 1)) == 0)
	{
		data.files = realloc(data.files, sizeof(*data.files) * (data.n_files + block_size));
	}
	data.files[data.n_files].type = type;
	data.files[data.n_files].name = strdup(name);
	data.files[data.n_files].display_name = malloc(strlen(name) + 4); // TODO: figure out how much this goes past
	strcpy(data.files[data.n_files].display_name, name);
	if (strlen(data.files[data.n_files].display_name) > LIST_WIDTH / data.largefont->w - 4)
	{
		strcpy(&data.files[data.n_files].display_name[LIST_WIDTH / data.largefont->w - 4], "...");
	}

	++data.n_files;
}


static void free_files()
{
	if (data.files)
	{
		for (int i = 0 ; i < data.n_files ; ++i)
		{
			free(data.files[i].name);
			free(data.files[i].display_name);
		}
		free(data.files);
	}

	data.files = NULL;
	data.n_files = 0;
	data.list_position = 0;
}


static int file_sorter(const void *_left, const void *_right)
{
	// directories come before files, otherwise case-insensitive name sorting

	const File *left = _left;
	const File *right = _right;

	if (left->type == right->type)
	{
		return strcasecmp(left->name, right->name);
	}
	else
	{
		return left->type > right->type ? 1 : -1;
	}
}


#ifdef WIN32

static void enumerate_drives()
{
	char buffer[1024] = {0};

	if (GetLogicalDriveStrings(sizeof(buffer)-1, buffer))
	{
		free_files();

		char *p = buffer;

		while (*p)
		{
			add_file(FB_DIRECTORY, p);
			p = &p[strlen(p) + 1];
		}

		debug("Got %d drives", data.n_files);

		data.selected_file = -1;
		data.list_position = 0;
		data.editpos = 0;

		qsort(data.files, data.n_files, sizeof(*data.files), file_sorter);
	}

}


static void show_drives_action(void *unused0, void *unused1, void *unused2)
{
	enumerate_drives();
}

#endif


static void parent_action(void *unused0, void *unused1, void *unused2)
{
	static File parent = {FB_DIRECTORY, "..", ".."};
	data.picked_file = &parent;
}


static void show_favorites_action(void *unused0, void *unused1, void *unused2)
{
	free_files();

	for (int i = 0 ; i < n_favorites ;++i)
		add_file(FB_DIRECTORY, favorites[i]);
}


bool filebox_is_favorite(const char *path)
{
	for (int i = 0 ; i < n_favorites ;++i)
		if (strcmp(favorites[i], path) == 0)
			return true;

	return false;
}


void filebox_add_favorite(const char *path)
{
	if (filebox_is_favorite(path)) return;

	favorites = realloc(favorites, (n_favorites + 1) * sizeof(char*));
	favorites[n_favorites] = strdup(path);

	++n_favorites;
}


void filebox_remove_favorite(const char *path)
{
	for (int i = 0 ; i < n_favorites ;++i)
		if (strcmp(favorites[i], path) == 0)
		{
			free(favorites[i]);

			if (n_favorites > 1)
			{
				memmove(favorites + i, favorites + i + 1, sizeof(char*) * (n_favorites - i - 1));
			}

			--n_favorites;

			return;
		}
}


void filebox_init(const char *path)
{
	debug("Loading filebox favorites (%s)", path);

	FILE *f = fopen(path, "rt");

	if (f)
	{
		while (!feof(f))
		{
			char ln[1024];
			if (fgets(ln, sizeof(ln), f))
			{
				strtok(ln, "\n");
				if (strlen(ln) > 0)
					filebox_add_favorite(ln);
			}
			else break;
		}

		fclose(f);
	}
}


void filebox_quit(const char *path)
{
	debug("Saving filebox favorites (%s)", path);

	FILE *f = fopen(path, "wt");

	if (f)
	{
		for (int i = 0 ; i < n_favorites ; ++i)
		{
			fprintf(f, "%s\n", favorites[i]);
			free(favorites[i]);
		}

		fclose(f);
	}
	else
	{
		warning("Could not write favorites (%s)", path);
	}

	n_favorites = 0;
	free(favorites);
	favorites = NULL;
}


static void add_favorite_action(void *_path, void *unused1, void *unused2)
{
	char *path = _path;
	filebox_add_favorite(path);
}


static void remove_favorite_action(void *_path, void *unused1, void *unused2)
{
	char *path = _path;
	filebox_remove_favorite(path);
}


static void ok_action(void *unused0, void *unused1, void *unused2)
{
	// Fake return keypress when field focused :)

	data.selected = true;
}


static void buttons_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect button;

	copy_rect(&button, area);

	button.w = strlen("Parent") * data.smallfont->w + 12;

	button_text_event(dest_surface, event, &button, data.gfx, data.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "Parent", parent_action, 0, 0, 0);
	button.x += button.w + 1;

#ifdef WIN32
	button.w = strlen("Drives") * data.smallfont->w + 12;
	button_text_event(dest_surface, event, &button, data.gfx, data.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "Drives", show_drives_action, 0, 0, 0);
	button.x += button.w + 1;
#endif

	button.w = strlen("Favorites") * data.smallfont->w + 12;
	button_text_event(dest_surface, event, &button, data.gfx, data.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "Favorites", show_favorites_action, 0, 0, 0);
	button.x += button.w + 1;

	button.w = strlen("OK") * data.largefont->w + 24;
	button.x = area->w + area->x - button.w;
	button_text_event(dest_surface, event, &button, data.gfx, data.largefont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "OK", ok_action, 0, 0, 0);
	button.x += button.w + 1;
}


static void pick_file_action(void *file, void *unused1, void *unused2)
{
	if (data.focus == FOCUS_LIST && data.selected_file == CASTPTR(int,file)) data.picked_file = &data.files[CASTPTR(int,file)];
	data.selected_file = CASTPTR(int,file);
	setfocus(FOCUS_LIST);
	if (data.files[data.selected_file].type == FB_FILE)
	{
		strncpy(data.field, data.files[CASTPTR(int,file)].name, sizeof(data.field) - 1);
		data.editpos = strlen(data.field);
	}
}


void window_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	bevel(dest_surface, area, data.gfx, BEV_MENU);
}


void title_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	const char* title = data.title;
	SDL_Rect titlearea, button;
	copy_rect(&titlearea, area);
	titlearea.w -= CLOSE_BUTTON - 4;
	copy_rect(&button, area);
	adjust_rect(&button, titlearea.h - CLOSE_BUTTON);
	button.w = CLOSE_BUTTON;
	button.x = area->w + area->x - CLOSE_BUTTON;
	font_write(data.largefont, dest_surface, &titlearea, title);
	if (button_event(dest_surface, event, &button, data.gfx, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_CLOSE, NULL, MAKEPTR(1), 0, 0) & 1)
		data.quit = 1;
}


void file_list_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content, pos;
	copy_rect(&content, area);
	adjust_rect(&content, 1);
	copy_rect(&pos, &content);
	pos.h = data.largefont->h;
	bevel(dest_surface,area, data.gfx, BEV_FIELD);

	gfx_domain_set_clip(dest_surface, &content);

	for (int i = data.list_position ; i < data.n_files && pos.y < content.h + content.y ; ++i)
	{
		if (data.selected_file == i && data.focus == FOCUS_LIST)
		{
			bevel(dest_surface,&pos, data.gfx, BEV_SELECTED_ROW);
		}

		if (data.files[i].type == FB_FILE)
			font_write(data.largefont, dest_surface, &pos, data.files[i].display_name);
		else
			font_write_args(data.largefont, dest_surface, &pos, "½%s", data.files[i].display_name);

		if (pos.y + pos.h <= content.h + content.y) slider_set_params(&data.scrollbar, 0, data.n_files - 1, data.list_position, i, &data.list_position, 1, SLIDER_VERTICAL, data.gfx);

		check_event(event, &pos, pick_file_action, MAKEPTR(i), 0, 0);

		update_rect(&content, &pos);
	}

	gfx_domain_set_clip(dest_surface, NULL);

	if (data.focus == FOCUS_LIST)
		check_mouse_wheel_event(event, area, &data.scrollbar);
}


void field_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content, pos;
	copy_rect(&content, area);
	adjust_rect(&content, 1);
	copy_rect(&pos, &content);
	pos.w = data.largefont->w;
	pos.h = data.largefont->h;
	bevel(dest_surface,area, data.gfx, BEV_FIELD);

	if (data.focus == FOCUS_FIELD)
	{
		int i = my_max(0, data.editpos - content.w / data.largefont->w);
		size_t length = strlen(data.field);
		for ( ; data.field[i] && i < length ; ++i)
		{
			font_write_args(data.largefont, dest_surface, &pos, "%c", data.editpos == i ? '§' : data.field[i]);
			if (check_event(event, &pos, NULL, NULL, NULL, NULL))
				data.editpos = i;
			pos.x += pos.w;
		}

		if (data.editpos == i && i < length + 1)
			font_write(data.largefont, dest_surface, &pos, "§");
	}
	else
	{
		char temp[100] = "";
		strncat(temp, data.field, my_min(sizeof(temp) - 1, content.w / data.largefont->w));
		font_write(data.largefont, dest_surface, &content, temp);
	}

	if (check_event(event, area, NULL, 0, 0, 0)) setfocus(FOCUS_FIELD);
}


static void path_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect button;
	copy_rect(&button, area);
	button.w = button.h = 14;
	button.x = area->x;
	button.y -= 5;

	if (filebox_is_favorite(data.fullpath))
	{
		button_event(dest_surface, event, &button, data.gfx, BEV_BUTTON_ACTIVE, BEV_BUTTON, DECAL_FAVORITE, remove_favorite_action, data.fullpath, 0, 0);
	}
	else
	{
		button_event(dest_surface, event, &button, data.gfx, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_UNFAVORITE, add_favorite_action, data.fullpath, 0, 0);
	}

	SDL_Rect text;
	copy_rect(&text, area);
	text.x += button.w + 2;
	text.w -= button.w + 2;

	font_write(data.smallfont, dest_surface, &text, data.path);
}


static int checkext(const char * filename, const char *extension)
{
	if (extension[0] == '\0') return 1;

	int i = strlen(filename);
	while (i > 0)
	{
		if (filename[i] == '.') break;
		--i;
	}

	if (i < 0) return 0;

	if (strcasecmp(&filename[i + 1], extension) == 0) return 1;

#ifdef __amigaos4__
	// check for old amiga-style file extensions (e.g. mod.amegas)

	if (i == strlen(extension) && strncasecmp(filename, extension, strlen(extension)) == 0)
	{
		return 1;
	}
#endif

	return 0;
}


static int populate_files(GfxDomain *domain, GfxSurface *gfx, const Font *font, const char *dirname, const char *extension)
{
	debug("Opening directory %s", dirname);

	char * expanded = expand_tilde(dirname);

	int r = chdir(expanded == NULL ? dirname : expanded);

	if (expanded) free(expanded);

	if (r)
	{
		warning("chdir failed");
		return 0;
	}

	getcwd(data.fullpath, sizeof(data.fullpath) - 1);
	getcwd(data.path, sizeof(data.path) - 1);

	size_t l;
	if ((l = strlen(data.path)) > ELEMWIDTH / data.smallfont->w)
	{
		memmove(&data.path[3], &data.path[l - ELEMWIDTH / data.smallfont->w + 3], l + 1);
		memcpy(data.path, "...", 3);
	}

	DIR * dir = opendir(".");

	if (!dir)
	{
		msgbox(domain, gfx, font, "Could not open directory", MB_OK);
		return 0;
	}

	struct dirent *de = NULL;

	free_files();

	slider_set_params(&data.scrollbar, 0, 0, 0, 0, &data.list_position, 1, SLIDER_VERTICAL, gfx);

	while ((de = readdir(dir)) != NULL)
	{
		struct stat attribute;

		if (stat(de->d_name, &attribute) != -1)
		{
			if (de->d_name[0] != '.' || strcmp(de->d_name, "..") == 0)
			{
				if ((attribute.st_mode & S_IFDIR) || checkext(de->d_name, extension))
				{
					add_file(( attribute.st_mode & S_IFDIR ) ? FB_DIRECTORY : FB_FILE, de->d_name);
				}
			}
		}
		else
		{
			warning("stat failed for '%s'", de->d_name);
		}
	}

	closedir(dir);

	debug("Got %d files", data.n_files);

	data.selected_file = -1;
	data.list_position = 0;
	data.editpos = 0;

	qsort(data.files, data.n_files, sizeof(*data.files), file_sorter);

	return 1;
}


int filebox(const char *title, int mode, char *buffer, size_t buffer_size, const char *extension, GfxDomain *_domain, GfxSurface *gfx, const Font *largefont, const Font *smallfont)
{
	domain = _domain;

	set_repeat_timer(NULL);

	memset(&data, 0, sizeof(data));
	data.title = title;
	data.mode = mode;
	data.picked_file = NULL;
	data.largefont = largefont;
	data.smallfont = smallfont;
	data.gfx = gfx;
	data.elemwidth = domain->screen_w - SCREENMARGIN * 2 - MARGIN * 2 - 16 - 2;
	data.list_width = domain->screen_w - SCREENMARGIN * 2 - MARGIN * 2 - SCROLLBAR - 2;
	strncpy(data.field, buffer, sizeof(data.field));

	if (!populate_files(domain, gfx, largefont, ".", extension)) return FB_CANCEL;

	for (int i = 0 ; i < data.n_files ; ++i)
	{
		if (strcmp(data.files[i].name, last_picked_file) == 0)
		{
			data.selected_file = i;

			// We need to draw the view once so the slider gets visibility info


			SDL_Event e = {0};

			draw_view(domain, filebox_view, &e);
			slider_move_position(&data.selected_file, &data.list_position, &data.scrollbar, 0);
			break;
		}
	}

	while (!data.quit)
	{
		if (data.picked_file)
		{
			set_repeat_timer(NULL);

			if (data.picked_file->type == FB_FILE)
			{
				if (mode == FB_OPEN || (mode == FB_SAVE && msgbox(domain, gfx, largefont, "Overwrite?", MB_YES|MB_NO) == MB_YES))
				{
					strncpy(buffer, data.picked_file->name, buffer_size);
					strncpy(last_picked_file, data.picked_file->name, sizeof(last_picked_file));
					free_files();
					SDL_StopTextInput();
					return FB_OK;
				}

				// note that after the populate_files() picked_file will point to some other file!
				// thus we need to check this before the FB_DIRECTORY handling below
			}
			else if (data.picked_file->type == FB_DIRECTORY && !populate_files(domain, gfx, largefont, data.picked_file->name, extension))
			{

			}
		}

		data.picked_file = NULL;

		SDL_Event e = { 0 };
		int got_event = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:

				set_repeat_timer(NULL);
				SDL_PushEvent(&e);
				free_files();
				SDL_StopTextInput();
				return FB_CANCEL;

				break;

				case SDL_KEYDOWN:
				{
					if (data.focus == FOCUS_LIST)
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_ESCAPE:

							set_repeat_timer(NULL);
							free_files();
							SDL_StopTextInput();
							return FB_CANCEL;

							break;

							case SDLK_KP_ENTER:
							case SDLK_RETURN:
							if (data.selected_file != -1) data.picked_file = &data.files[data.selected_file];
							else goto enter_pressed;
							break;

							case SDLK_DOWN:
							slider_move_position(&data.selected_file, &data.list_position, &data.scrollbar, 1);
							strncpy(data.field, data.files[data.selected_file].name, sizeof(data.field));
							data.editpos = strlen(data.field);
							break;

							case SDLK_UP:
							slider_move_position(&data.selected_file, &data.list_position, &data.scrollbar, -1);
							strncpy(data.field, data.files[data.selected_file].name, sizeof(data.field));
							data.editpos = strlen(data.field);
							break;

							case SDLK_TAB:
							setfocus(FOCUS_FIELD);
							break;

							default: break;
						}
					}
					else
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_TAB:
							setfocus(FOCUS_LIST);
							break;
						}
					}
				}
				// fallthru

				case SDL_TEXTINPUT:
				case SDL_TEXTEDITING:
					if (data.focus != FOCUS_LIST)
					{
						int r = generic_edit_text(&e, data.field, sizeof(data.field) - 1, &data.editpos);
						if (r == 1)
						{
							data.selected = true;
						}
						else if (r == -1)
						{
							free_files();
							SDL_StopTextInput();
							return FB_CANCEL;
						}
					}
					break;

				case SDL_USEREVENT:
					e.type = SDL_MOUSEBUTTONDOWN;
				break;

				case SDL_MOUSEMOTION:
					if (domain)
					{
						gfx_convert_mouse_coordinates(domain, &e.motion.x, &e.motion.y);
						gfx_convert_mouse_coordinates(domain, &e.motion.xrel, &e.motion.yrel);
					}
				break;

				case SDL_MOUSEBUTTONDOWN:
					if (domain)
					{
						gfx_convert_mouse_coordinates(domain, &e.button.x, &e.button.y);
					}
				break;

				case SDL_MOUSEBUTTONUP:
				{
					if (e.button.button == SDL_BUTTON_LEFT)
						mouse_released(&e);
				}
				break;
			}

			if (e.type != SDL_MOUSEMOTION || (e.motion.state)) ++got_event;

			// ensure the last event is a mouse click so it gets passed to the draw/event code

			if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && e.motion.state)) break;
		}

		if (data.selected)
		{
		enter_pressed:;
			struct stat attribute;

			char * exp = expand_tilde(data.field);

			int s = stat(exp ? exp : data.field, &attribute);

			if (s != -1)
			{
				if (!(attribute.st_mode & S_IFDIR) && mode == FB_SAVE)
				{
					if (msgbox(domain, gfx, largefont, "Overwrite?", MB_YES|MB_NO) == MB_YES)
					{
						goto exit_ok;
					}
					else
						data.selected = false;
				}
				else
				{
					if (attribute.st_mode & S_IFDIR)
						populate_files(domain, gfx, largefont, data.field, extension);
					else
					{
						goto exit_ok;
					}
				}
			}
			else
			{
				if (mode == FB_SAVE)
				{
					goto exit_ok;
				}
			}

			if (0)
			{
			exit_ok:;

				set_repeat_timer(NULL);
				strncpy(buffer, exp ? exp : data.field, buffer_size);
				strncpy(last_picked_file, "", sizeof(last_picked_file));

				if (mode == FB_SAVE && strchr(buffer, '.') == NULL)
				{
					strncat(buffer, ".", buffer_size);
					strncat(buffer, extension, buffer_size);

					char * exp = expand_tilde(buffer);

					int s = stat(exp ? exp : buffer, &attribute);

					if (exp) free(exp);

					if (s != -1 && mode == FB_SAVE)
					{
						if (msgbox(domain, gfx, largefont, "Overwrite?", MB_YES|MB_NO) == MB_NO)
						{
							data.selected = false;
							break;
						}
					}
				}

				free_files();
				if (exp) free(exp);
				SDL_StopTextInput();
				return FB_OK;
			}

			if (exp) free(exp);
		}

		if (got_event || gfx_domain_is_next_frame(domain))
		{
			draw_view(domain, filebox_view, &e);
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(5);
	}

	free_files();
	SDL_StopTextInput();
	return FB_CANCEL;
}
