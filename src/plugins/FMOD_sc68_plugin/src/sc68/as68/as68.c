/**
 * @ingroup   as68_devel
 * @file      as68.c
 * @author    Penne Vincent
 * @date      1993
 * @brief     68000 macro assembler - main program
 */

/*
 *                      as68 - 68000 macro assembler
 *                    Copyright (C) 1993 Vincent Penne
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* 
 * as68 - 68000 Macro Assembler version 2.00
 * Copyright (C) 1993 Vincent PENNE.
 *
 * Update to version 2.2 (2.1 was RiscPC version)
 * 1999 (BeN)jamin Gerard / SasHipA 1999
 *  - Posix compatible version
 *  - Some debug ( fast table, signed char, ds, dcb )
 *  - Adding command line control
 *  - Adding position independant check
 *  - Error messages tranlation & update
 *  - Added few comments
 *
 *  @bug  In relocation progress,  some relocation are not found !
 *  @bug  -P forces -R. @b org directive is disabled !!
 *  @bug  pea relatif PC, bugs with -P or -R
 *  @bug  symbol created by  xx = v are always lower case !
 *
 *  @par Disclaimer
 *
 *  This source code is quiet ugly ... Sorry...
 *
 *  But, it was old code. Vincent made it for his personnal use, in Atari ST
 *  and Amiga demos and games programming. Featuring a was very fast
 *  assembler linked by a printer port connection (4bit : PC rules), with
 *  special upload and exec directives. Notice that "Quick the thunder
 *  rabbit" released by Titus for Amiga 500/1200 and Amiga CD-32, was
 *  entirely made with it.
 *
 */

#include <config68.h>
//#define DEBUG _DEBUG
#define debug_printf printf
#define DEFAULT_CODE_BUFFER_SIZE 512 /* in Kb */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "as68.h"
#include "error.h"

static int assemble(char *, char *, char *);

filelist *file_list;
file *old_file;
char line_buffer[256], *plb;
char cur_string[256];
int cur_num;
uchar symbol_type;
position curpos, lastpos;
int cur_pass, n_errors, n_warnings, total_line, max_total_line,

    last_error_line;
char *end_line = "";
char *macro_args[MAX_MACRO_ARGS];
int n_macro_args;

/* code buffer */
uchar *code = NULL;
int buffer_size = DEFAULT_CODE_BUFFER_SIZE;

int pc, CurPC, max_pc, cur_rs, bss;
volatile int org;
int n_local, n_macro, cur_n_macro, n_label, n_equate;
struct fast_table *opcode_table[26], *register_table[26];
int size_table[] = { 1, 2, 4, 3 };
int load_adr, run_adr, no_load;

/* as68 command line option */
volatile int opt_relocatable = 0;
volatile int opt_verbose = 1;
volatile int opt_relwarn = 0;
volatile int opt_strip_bss = 0;

int relocation_table[MAX_REL], nb_rel;

FILE *ferr = NULL;
char cur_dir[1024];

#ifndef _WIN32
void itoa(int n, char *s, int sz)
{
  sprintf(s, "%d", n);
}
#endif

/* find directory , and copy into cur_dir with trailing '/' */
void find_curdir(char *n)
{
  char *slashpos;
  int l;

  if (n == NULL || (slashpos = strrchr(n, '/'), slashpos == NULL)) {
    cur_dir[0] = 0;
    return;
  }
  l = slashpos - n + 1;
  if (l > sizeof(cur_dir) - 1)
    l = sizeof(cur_dir) - 1;
  memcpy(cur_dir, n, l);
  cur_dir[l] = 0;

#if DEBUG
  printf("find_curdir(\"%s\") = \"%s\"/n", n, cur_dir);
#endif
}

/* ext = ".xxx" */
void chg_ext(char *filename, char *ext)
{
  char *s, *e;

  s = strrchr(filename, '/');
  e = strrchr(filename, '.');
  if (s >= e)                   /* Found '/' before '.' or  no '.' */
    strcat(filename, ext);
  else
    strcpy(e, ext);
}

void putlong(FILE * fd, long n)
{
  putc((n >> 24) & 0xff, fd);
  putc((n >> 16) & 0xff, fd);
  putc((n >> 8) & 0xff, fd);
  putc(n & 0xff, fd);
}

/******************************* MAIN PROGRAM ******************************/
static void help_message(void)
{
  printf("as68 V2.2 : MC68000 macro assembler\n"
         " (C)1993 Penne Vincent\n"
         " Posix compatible version by (BeN)jamin Gerard 1999\n"
         "Usage: as68 [-rRvVb####] <filein> [-o <fileout>] [-s <filesym1>] ...\n"
         "  -r            : Do not produce relocation table (default)\n"
         "  -R            : Produce relocation table\n"
         "  -p            : No position independant code check (default)\n"
         "  -P            : Warning position dependant addressing mode\n"
         "  -v            : Disable verbose\n"
         "  -V            : Enable Verbose (default)\n"
         "  -t            : Don't strip BSS (default)\n"
         "  -T            : Strip BSS\n"
         "  -b#####       : Change code buffer (##### is a number of Kbyte)\n"
         "  -s <filename> : Set relocation table filename (def:filein.rel)\n"
         "  -o <filename> : Set output filename (def:filein.bin) \n\n "
         );
}

/* if  <fname>!=NULL : Open <fname> in 'wb' mode
* else              : Open from created from <fin> with changed extension <ext>
*/
FILE *fopen_out(char **fname, char *ext, char *fin)
{
  static char tmp[1024];
  FILE *fd;

  if (*fname == NULL) {
    *fname = strncpy(tmp, fin, sizeof(tmp) - 4);
    chg_ext(tmp, ext);
  }
#if DEBUG
  printf("fopen_out(%s)\n", *fname);
#endif
  if (fd = fopen(*fname, "wb"), fd == NULL)
    fatal_error("Can't open %s in 'wb' mode", *fname);
  return fd;
}

int main(int na, char **a)
{
  int i=0, nxti=0, err=0;
  char *fin = NULL, *fout = NULL, *frel = NULL;

  /* Did they need help ??? */
  if (na < 2) {
    help_message();
    exit(1);
  }

  for (i = 1; i < na; i++) {
    if (!strcmp(a[i], "--help")) {
      help_message();
      exit(1);
    }
  }

  /* Standard error output */
  ferr = stderr;
  init_opcodes();

  fin = fout = frel = NULL;     /* set no input,output,reloc file */
  for (i = 1; i < na; i = nxti) {
    nxti = i + 1;
    if (a[i][0] == '-') {
      int j = 1;

      for (j = 1; a[i][j]; j++) {
        switch (a[i][j]) {
        case 'v':
          opt_verbose = 0;
          break;
        case 'V':
          opt_verbose = 1;
          break;
        case 'r':
          opt_relocatable = 0;
          break;
        case 'R':
          opt_relocatable = 1;
          break;
        case 'p':
          opt_relwarn = 0;
          break;
        case 'P':
          opt_relwarn = 1;
          break;
        case 'b':
          {
            int n = 0;

            ++j;
            if (!isdigit(a[i][j]))
              n = DEFAULT_CODE_BUFFER_SIZE;
            for (; a[i][j] && isdigit(a[i][j]); j++)
              n = n * 10 + a[i][j] - '0';
            buffer_size = n;
            break;
          }
        case 'o':
        case 's':
          if (nxti >= na) {
            printf("Missing Filename argument for switvh '%c' : Abort\n",
                   a[i][j]);
            exit(3);
          }
          if ((a[i][j] == 'o')) {
            fout = a[nxti++];
          } else {
            frel = a[nxti++];
          }
          break;
	case 'T':
	  opt_strip_bss = 1;
	  break;
	case 't':
	  opt_strip_bss = 0;
	  break;
        default:
          printf("Invalid switch option : '%c' : Abort\n", a[i][j]);
          exit(2);
        }
      }
    } else if (fin == NULL) {
      /* No input file ... get this one */
      fin = a[i];
    } else {
      /* Already an input file : assemble with current */
      err |= assemble(fin, fout, frel);
      fout = frel = NULL;
      fin = a[i];
    }
  }

  /* Process last file */
  if (fin != NULL) {
    err |= assemble(fin, fout, frel);
  }
  if (code != NULL) {
    free(code);
  }
  return err;
}

/* Assemble 68K source file <fin> into <fout> */
static int assemble(char *fin, char *fout, char *frel)
{
  volatile int opt_relocatable_save;
  int i;
  position start_pos;

  /* --- Becoz relocation could be check only when generated --- */
  opt_relocatable_save = opt_relocatable;
  opt_relocatable |= opt_relwarn;

  /* --- Alloc code buffer --- */
  code = (uchar *) realloc(code, 1024 * buffer_size);
  if (code == NULL)
    fatal_error("Can't alloc %d KB code buffer", buffer_size);

  /* --- Load source file --- */
  find_curdir(fin);
  if (!new_position(&start_pos, fin))
    fatal_error("Can't open '%s'", fin);

  /* --- reset globals --- */
  for (i = 0; i < 256; i++)
    word_table[i] = NULL;

  n_errors = 0;
  n_warnings = 0;
  n_label = 0;
  n_equate = 0;
  nb_rel = 0;
  last_error_line = -1;

  if (opt_relocatable)
    org = 0;

  printf("Assembling %s into %s position code\n", fin,
         opt_relocatable ? "relocatable" : "absolute");

  for (cur_pass = 1; (cur_pass <= 2 || (cur_pass == 3 && opt_relocatable))
       && !n_errors; cur_pass++) {

    if (opt_relocatable && cur_pass == 3)
      org = 0x12345678;         /* ??? */

#if DEBUG
    printf("PASS #%d org=$%x \n", cur_pass, org);
#endif

    n_local = 0;
    n_macro = 0;
    default_size = WS;
    max_pc = pc = 0;
    cur_rs = 0;
    bss=-1;
    total_line = 0;
    last_pass = TRUE;
    pass(&start_pos);

#if DEBUG
    printf("last_pass=%d\n", last_pass);
    printf("n_local=%d\n", n_local);
    printf("n_macro=%d\n", n_macro);
    printf("max_pc=%d\n", max_pc);
    printf("total_line=%d\n", total_line);
    printf("n_errors=%d\n", n_errors);
    printf("n_warnings=%d\n", n_warnings);
    printf("n_label=%d\n",n_label);
    printf("n_equate=%d\n",n_equate);
    printf("nb_rel=%d\n",nb_rel);
#endif

    if (cur_pass == 1)
      max_total_line = total_line;
    else if (max_total_line != total_line)
      internal_error("number of line assembled differs from previous pass");
  }

  opt_relocatable = opt_relocatable_save;

  if (!n_errors) {
    FILE *fd;

    /* Write relocation table : only if any entry or filename forced */
    if (opt_relocatable && (nb_rel || frel != NULL)) {
      fd = fopen_out(&frel, ".rel", fin); /* Can't return NULL !!! */
      printf("Writing relocation table : %s , %d entr%s\n",
             frel, nb_rel, nb_rel > 1 ? "ies" : "y");
      for (i = 0; i < nb_rel; i++) {
        putlong(fd, relocation_table[i]);
        code[max_pc + i * 4 + 0] = (relocation_table[i] >> 24) & 255;
        code[max_pc + i * 4 + 1] = (relocation_table[i] >> 16) & 255;
        code[max_pc + i * 4 + 2] = (relocation_table[i] >> 8) & 255;
        code[max_pc + i * 4 + 3] = relocation_table[i] & 255;
      }
      putlong(fd, -1);
      code[max_pc + i * 4 + 0] = 0xff;
      code[max_pc + i * 4 + 1] = 0xff;
      code[max_pc + i * 4 + 2] = 0xff;
      code[max_pc + i * 4 + 3] = 0xff;
      fclose(fd);
      max_pc += nb_rel * 4 + 4;
    }

    /* This test was for cross developpement ... no more used */
    if (!load_adr || no_load) {
      fd = fopen_out(&fout, ".bin", fin); /* Can't return NULL !!! */

      if (opt_strip_bss) {
	int save_max; 
	for (save_max=max_pc; max_pc>0 && !code[max_pc-1]; --max_pc);
	save_max = save_max - max_pc;
	printf("Strip BSS: %d byte%s removed\n",
	       save_max, save_max>1 ? "s" : "");
      }

      printf("Writing binary output : %s , %d byte%s\n",
             fout, max_pc, max_pc > 1 ? "s" : "");
      fwrite(code, 1, max_pc, fd);
      fclose(fd);
    }
  }

  /* Close error file is nor stderr */
  if (ferr != NULL && ferr != stderr)
    fclose(ferr);

  /* Clean up word table */
  for (i = 0; i < 256; i++) {
    delete_word_list(word_table[i]);
    word_table[i] = NULL;
  }

  if (!(n_errors + n_warnings)) {
    printf("Done...\n");
  }
  else {
    printf("%d error%s, %d warning%s\n\n",
           n_errors, n_errors > 1 ? "s" : "",
           n_warnings, n_warnings > 1 ? "s" : "");
  }
  return n_errors;
}

void fatal_error(char *format, ...)
{
  va_list list;

  va_start(list, format);
  printf("Fatal : ");
  vprintf(format, list);
  printf("\n");
  va_end(list);
  exit(0x68);
}

void internal_error(char *format, ...)
{
  va_list list;

  va_start(list, format);
  printf("Internal : ");
  vprintf(format, list);
  printf("\n");
  va_end(list);
  exit(0x66);
}

void error(char *format, ...)
{
  va_list list;

  va_start(list, format);
  if (last_error_line != lastpos.line) {
    fprintf(ferr, "%s(%d) : error pass:%d : ", lastpos.f->filename, lastpos.line, cur_pass);
    vfprintf(ferr, format, list);
    fprintf(ferr, "\n");
    n_errors++;
    last_error_line = lastpos.line;
  }
  va_end(list);
}

void warning(char *format, ...)
{
  va_list list;

  va_start(list, format);
  fprintf(ferr, "%s(%d) : warning pass:%d : ", lastpos.f->filename, lastpos.line, cur_pass);
  vfprintf(ferr, format, list);
  fprintf(ferr, "\"\n");
  n_warnings++;
  va_end(list);
}

void relatif_PC_warning()
{
  warning(warn_list[3]);
}

void error_expected(char *s)
{
  error("<%s> %s", s, error_list[30]);
}

void memory_error(char *where)
{
  fatal_error("Memory allocation failed (%s)",
              where == NULL ? "unknown" : where);
}

void syntax_error(void)
{
  error(error_list[1]);
}

void bit_lost_error(void)
{
  error(error_list[29]);
}

void error_already_def(char *s)
{
  error("<%s> %s", s, error_list[28]);
}

void error_undefined(char *s)
{
  error("<%s> %s", s, error_list[35]);
}

int new_position(position * p, char *c)
{
  p->line = 1;
  p->offset = 0;
  old_file = p->f = open_file(c);
  return (p->f != NULL);
}

int search_file(char *name)
{
  long l;
  FILE *f = fopen(name, "rb");

#if DEBUG
  printf("search(%s)=%p\n", name, f);
#endif

  if (f == NULL)
    return 0;
  fseek(f, 0L, SEEK_END);
  l = ftell(f);
  fclose(f);
  return (int) l;
}

/* --- Init file structure --- */

file *open_file(char *c)
{
  file *f;
  FILE *id;
  int size;

  size = search_file(c);
  if (!size)
    return NULL;
  f = (file *) malloc(sizeof(file));
  if (f == NULL)
    return NULL;
  strcpy(f->filename, c);
  f->nline = 0;
  id = fopen(c, "rb");
  if (id == NULL)
    return NULL;
  f->text = (char *) malloc(size);
  if (f->text == NULL)
    return NULL;
  f->size = fread(f->text, 1, size, id);
  fclose(id);
  return f;
}

char get_car()
{
  char c;

  c = curpos.f->text[curpos.offset];
  if (c == 13 || c == 10) {
    if (c != 10 && curpos.f->text[curpos.offset + 1] == 10)
      curpos.offset++;
    c = '\n';

    curpos.line++;
    total_line++;
  }
  curpos.offset++;
  return c;
}

uchar get_symbol()
{
  uchar c, *s;

  /* Kill space */
  for (c = *plb; isspace(c);)
    c = *++plb;

  /* Word type */
  if (isalpha(c) || c == '_') {
    s = cur_string;
    do {
      *s++ = c;
      c = *++plb;
    } while (isalnum(c) || c == '_');
    *s++ = 0;
/* #if DEBUG */
/*     printf("\015Word: %s\n", cur_string); */
/* #endif */
    return symbol_type = WORD_TYPE;
  }

  /* Decimal Number Type */
  if (isdigit(c)) {
    cur_num = 0;
    do {
      cur_num = cur_num * 10 + c - '0';
      c = *++plb;
    }
    while (isdigit(c));

/* #if DEBUG */
/*     printf("\015Number: %d\n", cur_num); */
/* #endif */
    return symbol_type = NUM_TYPE;
  }

  switch (c) {
    /* String */
  case '"':
    s = cur_string;
    c = *++plb;
    while (c && (c != '"' || *(plb + 1) == '"')) {
      *s++ = c;
      c = *++plb;
    }
    if (!c)
      error_expected("\\");
    else
      plb++;
    *s++ = 0;
/* #if DEBUG */
/*     printf("\015String: %s\n", cur_string); */
/* #endif */
    return symbol_type = STRING_TYPE;

    /* String */
  case '\'':
    s = cur_string;
    c = *++plb;
    while (c && (c != '\'' || *(plb + 1) == '\'')) {
      *s++ = c;
      c = *++plb;
    }
    if (!c)
      error_expected("\"");
    else
      plb++;
    *s++ = 0;
/* #if DEBUG */
/*     printf("\015String: %s\n", cur_string); */
/* #endif */
    return symbol_type = STRING_TYPE;

    /* Hexadecimal */
  case '$':
    cur_num = 0;
    c = *(++plb);
    if (!isalnum(c))
      syntax_error();
    else {
      do {
        if (c >= 'a')
          c -= 32;
        if (c >= 'A')
          c -= 'A' - 10 - '0';
        c -= '0';
        cur_num = cur_num * 16 + c;
        c = *++plb;
      }
      while (isalnum(c));
    }
/* #if DEBUG */
/*     printf("\015Number: %d\n", cur_num); */
/* #endif */
    return symbol_type = NUM_TYPE;

    /* Binary */
  case '%':
    cur_num = 0;
    c = *(++plb);
    if (c != '0' && c != '1')
      syntax_error();
    else {
      do {
        cur_num = cur_num * 2 + c - '0';
        c = *++plb;
      }
      while (c == '0' || c == '1');
    }
/* #if DEBUG */
/*     printf("\015Number: %d\n", cur_num); */
/* #endif */
    return symbol_type = NUM_TYPE;

  }

  if (c >= 128) {
/* #if DEBUG */
/*     printf("\015End_of_line\n"); */
/* #endif */
    return symbol_type = END_OF_LINE;
  }
  plb++;

/* #if DEBUG */
/*   printf("Character: %c\n", c); */
/* #endif */
  return symbol_type = c;
}

void get_line(void)
{
  char c = 0, *a, *b;
  int n;
  int p1, p2;

  p1 = p2 = 0;
  a = line_buffer;
  lastpos = curpos;
  while (curpos.offset < curpos.f->size && (c = get_car()) != '\n'
         && ((p1 & 1) || (p2 & 1) || c != ';')) {
    switch (c) {
      char expr[128];
      word *w;

    case '\\':
      n = get_car();
      if (n == '@') {
        *a++ = '_';
        itoa(cur_n_macro, a, 16);
        a = line_buffer + strlen(line_buffer);
      } else if (n >= '1' && n <= '9') {
        n -= '1';
        if (n >= 0 && n < MAX_MACRO_ARGS && macro_args[n]) {
          char *s = macro_args[n];

          while (*s)
            *a++ = *s++;
        }
      } else {
        *a++ = '\\';
        *a++ = n;
      }
      break;

    case '\'':
      p1++;
      *a++ = '\'';
      break;

    case '"':
      p2++;
      *a++ = '"';
      break;

    case '$':
      c = get_car();
      if (c == '(') {
        b = plb = expr;
        while (curpos.offset < curpos.f->size && (c = get_car()) != ')'
               && c != '\n')
          *b++ = c;
        *b++ = c;
        *b++ = (char)128;
        if (get_symbol() == WORD_TYPE) {
          w = search(cur_string);
          if (w && w->type == STRING) {
            b = w->pd.string;
            while (!!(*a++ = *b++));
            a--;
            if (get_symbol() != ')')
              syntax_error();
            break;
          }
        }
        n = expression();
        if (symbol_type != ')') {
          syntax_error();
          break;
        }
        itoa(n, a, 10);
        a += strlen(a);
      } else {
        *a++ = '$';
        *a++ = c;
      }
      break;

    default:
      *a++ = c;
    }
  }
  *a = 0;

  if (c == ';')
    while (curpos.offset < curpos.f->size && (c = get_car()) != '\n');

  plb = line_buffer;
  get_symbol();
}

void assemble_line()
{
  int n, f_local = FALSE;

  CurPC = pc;
  if (symbol_type) {
    word *w, *ww;
    struct fast_table *op;

    /* Local symbole */
    if (symbol_type == '.') {
      if (get_symbol() != WORD_TYPE)
        syntax_error();
      else {
        char a[128];

        strcpy(a, cur_string);
        sprintf(cur_string, "_%xL%s", n_local, a);
        f_local = TRUE;
      }
    }

    /* It's a symbole */
    if (symbol_type == WORD_TYPE) {
      if ((op = look_in_fast_table(cur_string, opcode_table))) {
        get_symbol();
        (*(op->function)) (op->value);
      } else {
        w = search(cur_string);
        if (!w) {
          ww = (word *) malloc(sizeof(word));
          if (!ww)
            memory_error("word");
          ww->name = strsav(cur_string);
          if (get_symbol() == ':')
            get_symbol();
          if (symbol_type == '=') {
            get_symbol();
            if (symbol_type == STRING_TYPE) {
              ww->type = STRING;
              ww->pd.string = strsav(cur_string);
              n_equate++;
              put_word(ww);
              get_symbol();
            } else {
              ww->type = VARIABLE;
              ww->pd.value = expression();
              if (notdef) {
                free(ww->name);
                free(ww);
              } else {
                n_equate++;
                put_word(ww);
              }
            }
          } else if (symbol_type == WORD_TYPE && streql(cur_string, "macro")) {
            position *p = (position *) malloc(sizeof(position));

            if (!p)
              memory_error("position");
            *p = curpos;
            ww->pd.pos = p;
            ww->type = MACRO;
            put_word(ww);
            skip_block();
          } else if (symbol_type == WORD_TYPE && streql(cur_string, "rs")) {
            int size;

            get_symbol();
            size = get_size(Word);
            ww->type = LABEL;
            ww->pd.value = ww->dd.value2 = cur_rs;
            cur_rs += expression() * size_table[size];
            put_word(ww);
            n_equate++;
          } else {
            if (!f_local)
              n_local++;
            ww->type = LABEL;
            ww->pd.value = ww->dd.value2 = pc + org;
            n_label++;
            put_word(ww);

            if (symbol_type == WORD_TYPE
                && (op = look_in_fast_table(cur_string, opcode_table))) {
              get_symbol();
              (*(op->function)) (op->value);
            }
          }
        } else {
          char *s;

          s = plb;
          if (get_symbol() == ':')
            get_symbol();
          switch (w->type) {
          case LABEL:
          case VARIABLE:
          case STRING:
            switch (symbol_type) {
            case '=':
              if (w->type == LABEL && cur_pass == 1)
                error(error_list[31]);
              get_symbol();
              if (w->type == STRING)
                free(w->pd.string);
              if (symbol_type == STRING_TYPE) {
                w->type = STRING;
                w->pd.string = strsav(cur_string);
                get_symbol();
              } else {
                w->type = VARIABLE;
                if (opt_relocatable && cur_pass > 2)
                  w->dd.value2 = expression();
                else
                  w->pd.value = w->dd.value2 = expression();
                if (notdef)
                  error(error_list[16]);
              }
              break;

            default:
              if (symbol_type == WORD_TYPE && streql(cur_string, "rs")) {
                if (cur_pass == 1)
                  error_already_def(cur_string);
                symbol_type = 0;
              } else {
                if (!f_local)
                  n_local++;
                if (cur_pass == 1)
                  error_already_def(cur_string);
                else if (opt_relocatable) {
                  w->dd.value2 = (w->pd.value = pc) + 0x12345678;
                } else
                  w->pd.value = pc + org;

                if (symbol_type == WORD_TYPE
                    && (op = look_in_fast_table(cur_string, opcode_table))) {
                  get_symbol();
                  (*(op->function)) (op->value);
                }
              }
            }
            break;

          case MACRO:
            if (symbol_type == WORD_TYPE && streql(cur_string, "macro")) {
              if (cur_pass == 1)
                error_already_def(cur_string);
              skip_block();
            } else {
              position pos = curpos;
              char **old_args;
              int old_n_args = n_macro_args, old_n_macro = cur_n_macro;

              n_macro++;
              cur_n_macro = n_macro;
              curpos = *w->pd.pos;
              old_args = (char **) calloc(sizeof(char *), n_macro_args);

              for (n = 0; n < n_macro_args; n++)
                old_args[n] = macro_args[n];
              n_macro_args = 0;
              for (n = 0; n < MAX_MACRO_ARGS; n++)
                macro_args[n] = 0;
              plb = s;
              while (*plb && isspace(*plb))
                plb++;
              while (*plb && *plb != ';') {
                char a[128], *s = a;

                while (*plb && *plb != ';' && *plb != ',')
                  *s++ = *plb++;
                if (*plb)
                  plb++;
                *s++ = 0;
                if (n_macro_args >= MAX_MACRO_ARGS)
                  error(error_list[32]);
                else
                  macro_args[n_macro_args++] = strsav(a);
              }

              assemble_block();

              for (n = 0; n < n_macro_args; n++)
                free(macro_args[n]);
              n_macro_args = old_n_args;
              for (n = 0; n < n_macro_args; n++)
                macro_args[n] = old_args[n];
              curpos = pos;
              cur_n_macro = old_n_macro;
            }
          }
        }
      }
    }

    switch (symbol_type) {
    case ';':
    case 0:
      break;

    default:
      syntax_error();
    }
  }
}

void assemble_block()
{
  while (curpos.offset < curpos.f->size) {
    get_line();
    if (symbol_type != ';' && symbol_type != '{' && symbol_type) {
      error(error_list[25]);
      return;
    }
    if (symbol_type == '{')
      break;
  }
  while (curpos.offset < curpos.f->size) {
    get_line();
    if (symbol_type == '}') {
      symbol_type = 0;
      plb = end_line;
      return;
    }
    assemble_line();
  }
  fatal_error(error_list[18]);
}

void skip_block()
{
  int n = 1;

  while (curpos.offset < curpos.f->size) {
    get_line();
    if (symbol_type != ';' && symbol_type != '{' && symbol_type) {
      error(error_list[25]);
      return;
    }
    if (symbol_type == '{')
      break;
  }
  while (curpos.offset < curpos.f->size) {
    get_line();
    switch (symbol_type) {
    case '{':
      n++;
      break;

    case '}':
      n--;
      if (n == 0) {
        symbol_type = 0;
        plb = end_line;
        return;
      }
      break;
    }
  }
  fatal_error(error_list[18]);
}

void pass(position * pos)
{
  curpos = *pos;
  while (curpos.offset < curpos.f->size) {
    get_line();
    if (symbol_type == WORD_TYPE && streql(cur_string, "end"))
      return;
    assemble_line();
  }
}
