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


#include "util/bundle.h"
#include "macros.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BS 1024*4
#define TEMP_NAME "makebundle.temp.$$$"
#undef main

int cwrite(void *ptr, int size, int num, FILE *f, Uint32 key)
{
	if (key == 0)
		return fwrite(ptr, size, num, f);
	else
	{
		int n = 0;
		for (int i = 0 ; i < num*size ; ++i)
		{
			Uint8 d = ((Uint8*)ptr)[i] ^ key;
			n += fwrite(&d, 1, 1, f);
			bnd_cycle_key(key);
		}
		
		return n/size;
	}
}

void copydata(FILE *dest, const char *src, off_t size, Uint32 key)
{
	FILE *f = fopen(src, "rb");
				
	if (f)
	{
		for (int p = 0 ; p < size ; p += BS)
		{
			unsigned char buffer[BS];
			
			int r = fread(buffer, 1, BS, f);
			if (cwrite(buffer, 1, r, dest, key) != r)
			{
				fputs("Error while writing data.\n", stderr);
				break;
			}
		}
		
		fclose(f);
		
		fprintf(stderr, "%u bytes written.\n", (Uint32)size);
	}
	else
	{
		fputs("Error while opening file.\n", stderr);
	}
	
	
}

int do_dir(char *dirname, char *basepath, FILE *bundle, FILE *data, Uint32 key)
{
	int nf = 0;
	
	DIR *dir = opendir(dirname);
	
	struct dirent *de;
	struct stat attribute;
	
	while ((de = readdir(dir)) != NULL)
	{
		char apath[500];
				
		sprintf(apath, "%s/%s", dirname, de->d_name);
		
		if( stat( apath, &attribute ) != -1 && ( attribute.st_mode & S_IFDIR ))
		{
			if (de->d_name[0] != '.')
				do_dir(apath, basepath, bundle, data, key);
		}
		else
		{
			fprintf(stderr, "%s: ", apath);
			
			if (attribute.st_size > 0)
			{
			
				Uint32 o = attribute.st_size;
				
				FIX_ENDIAN(o);
				
				char wpath[500];
				strcpy(wpath, apath + strlen(basepath) + 1);
				
				cwrite(wpath, sizeof(char), strlen(wpath)+1, bundle, key);
				cwrite(&o, sizeof(o), 1, bundle, key);
				
				copydata(data, apath, attribute.st_size, 0);
				
				++nf;
			}
			else
			{
				fputs("Skipping zero-length file.\n", stderr);
			}
		}
	}
	
	closedir(dir);
	
	return nf;
}

int main(int argc, char **argv)
{
	int first_path = 1;
	Uint32 key = 0;
	
	for (first_path = 1 ; first_path < argc ; ++first_path)
	{
		if (argv[first_path][0] == '-')
		{
			++first_path;
			if (first_path < argc)
			{
				for (int s =0 ; argv[first_path][s] ; ++s)
				{
					key = (key << 2) ^ ((Uint32)(argv[first_path][s])*13);
					bnd_cycle_key(key);
				}
					
				fprintf(stderr,"Using key %08x\n",key);
			}
			else
			{
				return 1;
			}
		}
		else break;
	}
	
	if (argc - first_path < 2)
	{
		fprintf(stderr, "Usage: %s [-k key] <dest bundle> <source paths ...>\n\n", argv[0]);
		return 1;
	}
	
	FILE *bundle = fopen(argv[first_path], "wb");
	
	if (!bundle)
	{
		fputs("Cannot create bundle file\n", stderr);
		return 1;
	}	
	
	fwrite(BND_SIG, strlen(BND_SIG), sizeof(char), bundle);
	
	Uint32 flags = key != 0 ? BND_FLAG_CRYPTED : 0;
	FIX_ENDIAN(flags);
	Uint32 num_files = 0;
	fwrite(&flags, 1, sizeof(flags), bundle); 
	
	Uint32 num_pos = ftell(bundle);
	
	fwrite(&num_files, 1, sizeof(num_files), bundle); //write dummy value
	
	FILE *data = fopen(TEMP_NAME, "wb");
	
	if (!data)
	{
		fputs("Cannot create temp file\n", stderr);
		return 1;
	}	
	
	for (int i = first_path+1 ; i < argc ; ++i)
		num_files += do_dir(argv[i], argv[i], bundle, data, key);
	
	fclose(data);
	
	int err_code = 0;
	
	if (num_files == 0)
	{
		fputs("No files found, no file created.\n", stderr);
		err_code = 1;
	}
	else
	{
		struct stat attribute;
		if (stat( TEMP_NAME, &attribute ) != -1)
		{
			fputs("Copying data to bundle: ", stderr);
			copydata(bundle, TEMP_NAME, attribute.st_size, key);
		}
		else
		{
			fputs("Error while opening temp file.\n", stderr);
		}
		
		fseek(bundle, num_pos, SEEK_SET);
		FIX_ENDIAN(num_files);
		fwrite(&num_files, 1, sizeof(num_files), bundle); //write real value
	}
	
	fclose(bundle);
	
	
	fputs("Cleaning up.\n", stderr);
	
	unlink(TEMP_NAME);
	
	return err_code;
}
