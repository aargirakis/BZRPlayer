/**
 * @ingroup   as68_devel
 * @file      as68.h
 * @author    Penne Vincent
 * @date      1993
 * @brief     68000 macro assembler - declarations
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
 

#define	TRUE	1
#define	FALSE	0

#define	MAX_MACRO_ARGS	10
#define	MAX_REL			(4096*3)

#define	WORD_TYPE		(128)
#define	NUM_TYPE		(129)
#define	STRING_TYPE (130)
#define	END_OF_LINE	(0)

#define	BS		0
#define	Byte	BS
#define	WS		1
#define	Word	WS
#define	LS		2
#define	Long	LS
#define	VS		3

#define	Alterable	00100
#define	Control		00200
#define	Memory		00400
#define	Data			01000

typedef struct file {
	char		filename[512];
	char		*text;
	int		size;
	int		nline;
} file;

typedef struct filelist {
	struct	file		*f;
	struct	filelist	*next;
} filelist;

typedef struct position {
	file	*f;
	int	offset;
	int	line;
} position;

/*
typedef enum wordtype {
	LABEL,
	VARIABLE,
	MACRO,
	STRING
} wordtype;
*/

#define wordtype char

#define LABEL			0
#define VARIABLE	1
#define MACRO			2
#define STRING		3

typedef unsigned char uchar;

typedef struct word
{
	struct word	*next;
	char        *name;
	wordtype		type;
	union
	{
		int      value;
		position *pos;
		char     *string;
	}pd;
	union
	{
		int n_calls;
		int value2;
	}dd;
} word;

extern struct fast_table
{
	struct fast_table	*letter[26];
	void (*function)();
	int value;
} *opcode_table[26], *register_table[26];

int	new_position(position *, char *);
file	*open_file(char *);
int	search_file(char *);

void	pass(position *pos), assemble_line(void), skip_block(void), assemble_block(void);
void	init_opcodes(void);
int	expression(void), terme(void), primaire(void);
struct fast_table *look_in_fast_table(char *s, struct fast_table **pft);
uchar get_symbol(void);
int streql(char *, char *);
int get_size(int);
extern FILE *ferr;

/* --- Function included in as68.c --- */

void chg_ext(char *, char *);
void get_line( void );

/* Error messages */

void fatal_error(char *,...);
void internal_error(char *, ...);
void memory_error(char *);

void error(char *,...);
void warning(char *,...);
void relatif_PC_warning();

void error_expected(char *s);
void error_undefined(char *s);
void syntax_error(void);
void bit_lost_error(void);

/* --- Function included in word.c --- */

int checksum(char *);
word *search(char *);
void put_word(word *);
void delete_word(word *);
char *strsav(char *);
void delete_word_list(word *);

/* --- Function in expression.c --- */

int primaire( void );
int sous_expression(void );
int expression( void );

/* --- Function in opcode.c --- */

int streql(char *, char *);
void tstb(int);
void tstw(int);
void genB(int);
void genW(int);
void putW(int , int);
void gen24(int);
void genL(int);
int getC(int);
int getN(int);
int getL(int);
void adressing_mode_error(void);
void size_error(void);
int tt(int , int , int , int );
void erreur(int);
void tstvirg(void);
void tst(char);
int get_size(int);
int isreg(char *);
int tstopt( void );
void addopt(char *);
int reglist( void );
int op(int , int );
void init_opcodes( void );

/* ----------------------------- */
extern	position	curpos, lastpos;
extern	filelist	*file_list;
extern	int   cur_pass, n_local, n_macro, cur_n_macro, n_label;
extern	uchar symbol_type;
extern	char	line_buffer[256], *plb;
extern	char	cur_string[256];
extern	int		cur_num;
extern	word	*word_table[256];
extern	char	notdef, last_pass;
extern	uchar	*code;
extern	int		pc, CurPC, buffer_size, max_pc, cur_rs;
extern  volatile int	org;
extern	int		default_size;
extern	int		size_table[];
extern	int		load_adr, run_adr;
extern	int		relocation_table[MAX_REL], nb_rel;
extern	volatile int	opt_relocatable;
extern	char	cur_dir[];
