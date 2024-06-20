/** @defgroup info68 info68
 *  @{
 *
 * Get and format information from sc68 files.
 *
 * @section author AUTHOR
 *
 *   Copyright (C) 2001-2003 Benjamin Gerard <ben@sashipa.com>
 *
 *   This program is free software.
 *
 * @section date DATE
 *
 *   2001
 *
 * @section synopsis SYNOPSIS
 *
 *   info68 [option ...] <file.sc68> [command ...] 
 *
 * @section description DESCRIPTION
 *
 *   info68 is a program to display information on an sc68 file in
 *   a free format.
 *
 * @section options OPTIONS
 *
 *  General options.
 *
 *   @arg @b --help        Display this message and exit.
 *   @arg @b --version     Display build date and exit.
 *   @arg @b --build       As --version.
 *   @arg @b -o file       Change output to file (- is stdout)
 *   @arg @b --output file As -o.
 *
 * @section commands COMMANDS
 *
 *   All COMMANDS excepted TRACK-LIST display something. Unkwown commands
 *   are skipped with a warning.
 *
 * @subsection diskcommands DISK-COMMANDS
 *
 *  These commands display disk information.
 *
 *   @arg @b -#          Number of tracks
 *   @arg @b -?          Default track
 *   @arg @b -N          Disk name
 *   @arg @b -A          Default track author name
 *   @arg @b -C          Default track composer name
 *   @arg @b -T          Disk time in sec
 *   @arg @b -Y          Formated disk time. Format "TT MM:SS"
 *   @arg @b -H          All tracks ORed hardware flags (see -h)
 *
 * @subsection tracklist TRACK-LIST
 *
 *   @arg @b -DIGIT[[,DIGIT]|[-DIGIT]]
 *             track-list executes following track-commands for all listed
 *             tracks until another track-list is encountered or end of
 *             command line.
 *
 *             Track are base 1 numbered.
 *             0 is substituted to the number of tracks.
 *             This command is a kind of loop.
 *
 *             @e e.g: "-1,5,[4-6]" works with tracks 1,5,4,5 and 6
 *                     in this order.
 *
 * @subsection trackcommands TRACK-COMMANDS
 *
 *   Following commands are applied to current track.
 *
 *   @arg @b -%       Track number
 *   @arg @b -n       Track name
 *   @arg @b -a       Author name
 *   @arg @b -c       Composer name
 *   @arg @b -r       Replay name
 *   @arg @b -t       Time in sec
 *   @arg @b -y       Formated time. Format "TT MM:SS"
 *   @arg @b -f       Replay frequency
 *   @arg @b -@       Load address
 *   @arg @b -h       Hardware flags [YSA] uppercase means activated.
 *                    Y:YM-2149
 *                    S:STE
 *                    A:Amiga
 *
 * @subsection misccommand MISC-COMMANDS
 *
 *   @arg @b -L        Display a newline character
 *   @arg @b --STRING  Display "-STRING"
 *   @arg @b STRING    Display "STRING"
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
