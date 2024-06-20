/**
 * @defgroup sc68 sc68
 * @{
 *
 * sc68 command line player.
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
 *   sc68 [options] <input.sc68>
 *
 * @section description DESCRIPTION
 *
 *   sc68 is an Atari ST and Amiga music emulator.
 *
 *   This program loads and plays a sc68 disk file (.sc68).
 *   Audio data is written to standard output.
 *   PCM format is stereo signed 16 bit stereo machine endian.
 *
 * @section options OPTIONS
 *
 *   @arg @b --help          Display this message and exit.
 *   @arg @b --version       Display sc68 version x.y.z and exit.
 *   @arg @b --quiet         Do not display music information.
 *   @arg @b --track=[track] Choose track to play [0=all tracks].
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
