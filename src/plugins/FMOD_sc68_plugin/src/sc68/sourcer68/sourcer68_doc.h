/**
 * @defgroup  sourcer68 sourcer68
 * @{
 *
 * 68000 disassembler.
 *
 * @section author AUTHOR
 *
 *   Copyright (C) 1998-2003 Benjamin Gerard <ben@sashipa.com>
 *
 *   This program is free software.
 *
 * @section date DATE
 *
 *   2003
 *
 * @section synopsis SYNOPSIS
 *
 *   sourcer68 [options] <input>
 *
 *
 * @section description DESCRIPTION
 *
 *   sourcer68 is a 68000 disassambler. Input file can be either BINARY,
 *   TOS program (Atari .prg .tos ...) or SC68 file. The dissassembler
 *   starts a disassemble pass at the beginning of the input depending on
 *   its type.
 *   @arg @b BINARY
 *           entry is offset +0.
 *   @arg @b TOS
 *           entry is the program entry point.
 *           By default TOS program are relocated.
 *   @arg @b SC68
 *           3 entries are offset +0 +4 and +8 corresponding to sc68
 *           replay convention.
 *           External replay is loaded if needed. Specify replay-path
 *           with --replay= or with environment varaiable.
 *
 * @subsection pass DISASSEMBLE PASS
 *
 *   A disassemble pass starts at an ENTRY point. The disassembler will try
 *   to follow program branchs. It should add every needed symbols.
 *   There is a heuristic to try to find jump tables
 *   (sequential branch instruction).
 *
 * @section options OPTIONS
 *
 *   @arg @b --help                    Display this message and exit.
 *   @arg @b --reloc=[yes|no|auto]     TOS relocation (default:auto).
 *   @arg @b --replay=PATH             Force replay path.
 *   @arg @b --entry=[ENTRY-LIST]      Set disassembly entry points.
 *   @arg @b --tab=[STRING]            Set tabulation string.
 *
 * @subsection  entrylist ENTRY-LIST
 *
 *    ENTRY-LIST := ENTRY[,ENTRY...]
 *
 *    An @b ENTRY-LIST is an ordered coma (',') separated list of @b ENTRY
 *    points which define disassembler start points.
 *
 * @subsection  entry ENTRY
 *
 *    ENTRY := [l][+]integer
 *
 *    @arg @b l        Get long at the effective address (indirection).
 *                     'l' options could by use more than once. (eg. ll0x440).
 *    @arg @b +        Effective address is file start + integer.
 *    @arg @b integer  Number ("C" format: 0x for hexa, 0 for octal)
 *
 * @section environment ENVIRONMENT VARIABLES
 *
 *   The @b SC68_DATA variable overrides the default shared data path.
 *   By default it is set to @e $datadir/sc68 where @e $datadir depends on
 *   the installation options. In most case this is @e /usr/local/share/sc68
 *   or @e /usr/share/sc68.
 *
 *   The @b SC68_HOME variable overrides the default user data path.
 *   By default it is set to $HOME/.sc68.
 *
 * @section bugs BUGS
 *
 *   None has been reported. Report to @e bug@sashipa.com.
 *   
 * @section seealso SEE ALSO
 *
 *   @e as68, @e debug68, @e info68, @e sc68, @e sourcer68, @e unice68
 *
 * @}
 */
