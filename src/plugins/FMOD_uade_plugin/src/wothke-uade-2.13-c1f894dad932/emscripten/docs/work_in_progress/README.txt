There are certain sound files that UADE 2.13 (and maybe newer releases as well) do
not support. Examples are eagleplayers that load additional shared libraries,
use Tasks, additional Processes, Signal, etc.

I have tried to fix some of these flaws in my UADE 2.13 derivative here.

This file just serves to remind me of the current status.


work-in progress
----------------

- "G&T Game Systems": while most songs seem to play fine, the song "DUX.Carver 1"
  completely crashes the browser after a few seconds of playback.

- UADE's original support for the below formats is flawed (see test songs for the 
  respective formtat):
  
  * ProPacker1.0
  * Promizer1.8a
  
  might be fixed by using converter instead

"Fixed" players
---------------

- Some songs seem to depend on WantedTeam.bin - without actually shipping that file
  with the song. I therefore added that file as a resource that is always loaded
  locally. (testcase: JesperOlsen)

- audio.device: added basic support suitable to make some of the easier
  test cases work. known limitations: fixed & added players: "G&T Game Systems"
  and "Digital Sound Creations". The audio.device impl is still incomplete
  and there may well be songs that will not play perfectly.

- Added PutMsg and softInt handling to fix: "Music-X Driver" (and added player)

- PlayAY: I added "dummy"/minimal exec.library and dos.library functions that had not
  been implemented yet. The new replacement LoadSeg impl might also have helped
  since separate BSS segments are being used. Added missing A4 (register) initialization
  that seems to be expected by certain DTG_InitPlayer/DTG_InitSound players (where are
  those bloody EaglePlayer APIs actually documented?!). ZXAYST11, ZXAYAMAD and
  ZXAYSTRC now work. ZXAYEMUL is moved to the "abandoned pile" - see below.

- added EaglePlayers (these seem to work fine in spite of not having been bundled
  with UADE 2.13): Andrew Parton, Ashley Hogg, Janne Salmijarvi Optimizer,
  Kim Christensen, Mosh Packer, Nick Pelling Packer, TimeTracker, Titanics Packer,
  UFO

- MusicMaker4+8 updated to: https://aminet.net/package/mus/edit/MMV8_Complete

- FaceTheMusic: The added audio.device and fixed handling for DTP_StartInt fixed
  this.

- Custom: I managed to fix songs like Archon*, ArcticFox, MarbleMadness, OneOnOne, 
  SevenCitiesOfGold, SkyFox, WorldTourGolf by adding respective MT and audio.device support.

- Stonetracker: added generic library loading (used here for "stonepacker.library"
  and "stoneplayer.library). Replaced UADE's flawed original LoadSeg implementation with 
  something that seems to work better. Also added dummy "utility.library" and "graphics.library"
  to provive used functionalities. Fixed flaws in UADE's original interrupt handling impl 
  and added dummy impls for used additional exec_lib functions.
  The audio.device based Stonetracker player version 1.2 (which uses stoneplayer.library)
  now works fine. The noteplayer based Stonetracker player version 1.6 also plays - but
  it sounds horribe due to the flaws of UADE's noteplayer impl.

- increased "chipmem_size" to 16 in "uaerc"; this fixes playback of large songs 
  (e.g. "Dark Helmets", etc)

Abandoned players
-----------------

These currently also don't work in UADE - but I don't care..

- DigiBoosterPRO: introduces a long list of shared library dependencies that might
  just trigger a ton of extra work. But primarily it seems to base its audio output
  on AHI.library: and since AHI covers more than only the built-in Amiga soundchip,
  it would probably more much more work to implement it than audio.device
  => OpenMPT already covers this format - so there is no point to waste any time on this

- WizardOfSound: did not find any songs except the examples shipped on the respective
  editor's disks. Copying those (including the Instruments) fresh from the emulator
  did not prove to be successful. The player aborts with module check fail.. I don't
  know what this player expects but it just doesn't seem to be worth the trouble.

- 669: UADE warns "noteplayer error: multichannel song" and the songs then seem to
  play much too fast - rendering the player unusable. I don't know what the respective
  "multichannel song" limitation of noteplayer might be. but fixing this would be
  much easier for somebody with a respective "noteplayer" implementation background..
  => XMP already covers this format - so there is no point to waste any time on this

- SymphoniePro: "unknown amiga volume (CON:20/20/300/40/Symphonie - Error Message);"
  => just too lazy to unvestigate this

- RonKlaren: none of the 3 players that I found plays any of the bundled test
  songs.. however the regular "CustomMade" does play the "rkb." file.
  -> since most songs probably also exist in the "rkb" form this probably isn't
  worth the trouble

- PaulTonge: supposedly a "modified version of Sound Images" player.. it uses blitter
  and disk related APIs (via the WantedTeam.bin replay) that are not currently implemented
  in UADE. this could potentially be fixed. But with a grand total of 2 games actually
  using this (none of them available in modland nor unexotica.org) this just isn't worth
  the trouble.

- PlayAY/ZXAYEMUL: Among the PlayAY players this is the hard nut: This player expects
  to run a Z80 emulator as a separate additional process - that then sends messages
  to the waiting player process to make it execute some code/update the audio registers.
  => it is questionable if that interaction could be hacked to just run synchronously
  within one process.. and if that then still could be integrated into UADE's playloop
  somehow.
  => this probably isn't worth the trouble since NEZplug already plays this format
  (unless the quality of the AY soundchip emulation here was actually better - who knows?)

- IFF-SMUS: Player tries to load a EP_SMUS.Config file that is not even provided with the
  distributions that this player comes with (was this ever even tested?). UADE's soundcore
  then crashes due to some unimplemented exec-function. However SonixMusicDriver seems to
  be a more recent functional replacement.

- IFF 8SVX (8-bit sampled audio voice): There seem to be "PCM" and "Fibonacci" variants of 
  this format (which can be played in Deliplayer2). The Eagleplayer fails in UADE since it 
  relies on multiple features that are not available here: Most importantly it needs 
  exec_lib/CreateNewProc - and multi-processing isn't implemented in UADE. It then uses 
  various dos_lib functions that are also not available (_LVOLock, _LVOOpenFromLock, 
  _LVOUnLock, _LVODupLock,_LVODelay). Then the player uses Eagleplayer's "Amplifier" 
  API and based on UADE's "warning: amplifier used" it seems more than likely that 
  support for that API is probably not ready yet neither. Finally there are minor issues - 
  like the used DTP_Check1 API that isn't handled in UADE (see patched sources in 
  amigasrc\players\iff). The few example songs that I found just don't seem to be worth the trouble.

- IFF EMOD: The comment in the file mentions "QuadraComposer" and this is already handled
  by the respective player.

- FastTracker II: UADE prints "warning: amplifier used" which probably means that API hasn't
  been properly implemented yet. Also many ".xm" files are larger then what UADE currently
  considers the maximum RAM available. Since XMP as well as MPT already play this format
  there is no point to add respective support in UADE.

- AIFF: existing AIFF player from EaglePlayer2.06 crashes with "ended: score died". Mainstream
  programs like WinAMP and XMplay can already be used to play these. No point in adding this.


Alias names
-----------

sometimes different names seem to be used for the "same" player - or the respective
new player just replaces the old one:

Tiny Williams => SoundImages
IFF-SMUS => SonixMusicDriver
Vectordean => RichardJoseph
Grouleff => EarAche
THX/THX II => Abyss' Highest eXperience


Links
-----
http://wt.exotica.org.uk/players.html
http://wt.exotica.org.uk/examples.html