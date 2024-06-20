# WebUADE+

Copyright (C) 2014-2023 Juergen Wothke

An online demo can be found here: https://www.wothke.ch/webuade++/


This is a JavaScript/WebAudio plugin of UADE (Unix Amiga Delitracker Emulator). This plugin is designed to
work with my generic WebAudio ScriptProcessor music player (see separate project).



WebUADE+ has some added improvements compared to the original UADE 2.13:

- Added built-in conversion for "exotic" mod file formats so they play with the existing PTK-Prowiz player.

- Impoved soundcore - see https://bitbucket.org/wothke/uade-2.13/src/master/emscripten/docs/work_in_progress/README.txt
so that this version now handles additional music files that the original UADE doesn't: Andrew Parton, Ashley Hogg,
Binary Packer, Custom modules (various), Digital Illusion, Devils' replay, Digital Sound Creations, EurekaPacker (improved),
FaceTheMusic, FuchsTracker, G&T Game Systems, GnoiPacker, GnuPlayer GPMO, HCD-protector, Janne Salmijarvi Optimizer,
Kim Christensen, Module-Patterncompressor (PMd3), Mosh Packer, Music-X Driver, NewtronPacker 1.0/2.0, Nick Pelling Packer,
Noise From Heaven (Ben replay), Noiserunner, PERFSONG Packer, PERFSNG2 Packer, PlayAY (except ZXAYEMUL),
PolkaPacker (improved), Power Music (improved), PreTracker, Promizer1.0b, SlamTilt, StartTrekker pack (improved),
Stone Arts Player, StoneTracker, TheDarkDemon, The Player 2.2a (P22A), Timetracker (TMK Replay), Titanics Packer,
UFO, UNIC (support), WantonPacker.

- support for original Quartet ST files (without the Amiga renaming garbage) as well as the (potentially packed) *.4q variant

- automatic handling of packed Amiga music files (MMCMP, PowerPacker, SQSH, S404)

- separate output of the 4 MOS 8364 channels (for use in realtime stream visialization during playback)


## Project structure

- `src` contains the "original" UADE C code (patches are usually identified via EMSCRIPTEN ifdefs) - with added
sub-folders `decrunch`, `prowiz` and `unice68`

- `amigasrc` contains the extensively changed Motorola 68k source code parts used in the emulation, i.e. for
`score` (basically the Amiga Kickstart replacement) and the various Eagleplayers. (The resulting binaries
can be found in `emscripten/htdocs/uade/...`)

- `emscripten` contains the Web port specific code - see additional `README` in that folder.


## Dependencies
The current version requires version >=1.2.1 of my https://bitbucket.org/wothke/webaudio-player/



## KNOWN LIMITATIONS

The original UADE project is currently on version 3.x, i.e. the version used as a base here is somewhat dated. But
unfortunately the new 3.x version still seems to suffer from the same design flaws as the old version: with regard to
reusability of the actual emulation / audio generation logic (hard-wired IPC / multi-threading dependencies, etc)

I might update my player eventually (if that garbage ever gets cleaned up in the UADE 3.x codebase) but I don't
care having to once again waste my time to make all those patches that I had to make just to get the 2.13 version
to work in a Web context.



## LICENSE

UADE uses different licences (mainly GPL and LGPL) for different parts of the implementation. I did not check
if that licensing actually holds up to closer scrutiny - or if there are already parts that are incompatible
with GPL. (Which unfortunately is the case in so many "open source" projects that don't understand the actual
implications of GPL..). In any case, any additions that I made and that I do hold the copyright of are licensed
such that they can be back ported into the "official UADE" project. Exception: the amigasrc\score\exec\test folder
contains some experimental test code that specifically MUST NOT be used/activated in regular distributions
due to its incompatibilty with GPL (see respective switch USE_TEST_ONLY_CODE in score.s)!

## Credits

WebUADE+ is based on:

- UADE version 2.13: http://zakalwe.fi/uade

>		Main authors
>
>		 - Heikki Orsila <heikki.orsila@iki.fi>
>		 - Michael Doering <mldoering@gmx.net>
>
>		Subsystems
>
>		 - Antti S. Lankila <alankila@bel.fi> for filter emulation and sound effects
>		 - Claudio Matsuoka and Hipolito Carraro Jr for module decruncing code in
>		   uade 1.xy.
>			jah@fh-zwickau.de for unsqsh
>			Sipos Attila for unsqsh checksum
>			Olivier Lapicque for mmcp
>			Marc Espie (old depack.c)
>		 - Harry "Piru" Sintonen <sintonen@iki.fi> for AmigaOS and MorphOS port
>		 - Martin Blapp for configure fixes and enhancements from FreeBSD project
>		 - Michael S. Baltaks for Mac OS X port
>		 - Nicolas A. Mendoza (part of the AmigaOS port)
>		 - Stuart 'kyzer' Caie for Mac OS X port and powerpacker decruncher
>		   http://www.kyz.uklinux.net
>
>		Everyone from UAE project. See doc/UAE-CREDITS.
>
>		Eagleplayers
>
>		 - Don Adan / Wanted Team
>		 - Eagleeye and Buggs from Defect (Eagleplayer project authors)
>		 - Nicolas Frank <gryzor@club-internet.fr> for PTK-Prowiz
>		 - Andy Silva <silva@psi5.com> for his replayers
>		 - Bartman and Dexter from Abyss for AHX v2 replay routine:
>		   http://www.the-leader-of-the-eighties.de
>		 - Brian Postma <b.postma@hetnet.nl> for Brian's Soundmonitor player
>		   http://www.homepages.hetnet.nl/~brianpostma
>		 - Nicolas Pomared <npomarede@kaptech.com> for MYST/YMST replayer
>		 - Sean Connolly for EMS V6 replay: http://www.cosine-systems.com/
>		 - Stephen Mifsud (Malta) <teknologik@technologist.com> for Darius Zendeh
>		   replayer. http://www.marz-kreations.com
>		 - Sunbeam/Shelter for his replayers
>		 - Paul v.d. Valk for Medley replay routine
>		 - Tap & Walt for digibooster source
>		   http://www.highantdev2.de/dbpro/index.php
>		 - The Welder / Divine for protracker replay routine
>
>		 - Everyone else whose Eagleplayer plugins we use. Respective authors of
>		   eagleplayer plugins can be found from inside the plugin.


- amigadepacker: https://gitlab.com/heikkiorsila/amigadepacker

- ProWizard 1.71: http://asle.free.fr/prowiz/