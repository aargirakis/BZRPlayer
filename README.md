# BZR Player 2

BZR Player 2 is an audio player for Windows with the primary goal being able to play a lot of different file formats.\
It is developed in C++ and QT. The sound engine is based on FMOD. The first version was released 12-Apr-2008.\
The last 1.x version was released 2019-Apr-08.\
This is the beginning of the new 2.x version which is coded pretty much from scratch.\
Please add features and bugs here on GitHub.

## How To Build

### Windows (#TODO msys2 gdb?):

**MSYS2** with following packages (install as shown) is required:\
`pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-qt5-base mingw-w64-i686-qt5-svg openssl-devel make`

 - Open the MSYS2 **mingw32** command prompt
 - Go to your project dir (take in mind Unix-style paths are required)
 - Inside the project directory create the cmake build directory with name `cmake-build-[debug|release]`,
 - then enter it and execute:\
`cmake -DCMAKE_PREFIX_PATH=/mingw32 -DCMAKE_BUILD_TYPE=[Debug|Release] -G Ninja .. && ninja`

#### build example:
```
cd /c/BZRPlayer
mkdir cmake-build-release</strong>
cd cmake-build-release</strong>
cmake -DCMAKE_PREFIX_PATH=/mingw32 -DCMAKE_BUILD_TYPE=Release -G Ninja .. && ninja
```

### Linux cross-compilation:

Dockerized cross-compilation toolchain is provided: just execute `run.sh` from the **
docker** directory, eventually setting `BUILD_TYPE=Release` if needed.

If you also want to run BZR2 after the build, then set `RUN_BZR2=1` (**wine** is required).

----

Official page is http://bzrplayer.blazer.nu

![bzr2 0 27](https://user-images.githubusercontent.com/10993634/201359947-2633341d-9ff6-4a59-bb9e-ce1794df9cba.png)

----


## SUPPORTED FORMATS

### Using Libsidplayfp
Commodore 64 SID, PSID

### Using Audio File Library
Amiga IFF/8SVX\
Audio Visual Research\
Berkeley/IRCAM/CARL\
Compressed AIFF\
NeXT .snd\
NIST SPHERE\
SampleVision\
Sun .au

### Using ASAP
Atari systems using POKEY sound chip\
Chaos Music Composer\
Chaos Music Composer -3-4-\
Chaos Music Composer -Rzog-\
Delta Music Composer\
DoublePlay Chaos Music Composer\
Music ProTracker\
Raster Music Tracker\
Stereo Double Chaos Music Composer\
Theta Music Composer 1.x 4-channel\
Theta Music Composer 1.x 8-channel\
Theta Music Composer 2.x

### Using Game Music Emu
AY - ZX Spectrum, Amstrad CPC\
GBS - Nintendo Game Boyr\
GYM - Sega Genesis, Mega Drive\
HES - NEC TurboGrafx-16, PC Engine\
KSS - MSX Home Computer, other Z80 systems\
NSF - NES Sound Format\
NSFE - NES Sound Format Extended\
SPC - Super Nintendo, Super Famicom\
VGM - Video Game Music File\
VGZ - Compressed Video Game Music File\
RSN - RAR archive with SPC songs

### Using HivelyTracker
AHX\
HivelyTracker

### Using KB Media Player
S98

### Using Ken's Digital Music
Ken's Digital Music

### Using libpac
SBStudio PAC

### Using LibV2
Farbrausch V2M

### Using Organya
Organya

### Using UADE
ActionAmics\
Activision Pro (MartinWalker)\
Alcatraz_Packer\
AM-Composer\
Anders Øland\
Andrew Parton\
Art And Magic\
ArtOfNoise-4V\
ArtOfNoise-8V\
Ashley Hogg\
BeathovenSynthesizer\
Ben Daglish SID\
BladePacker\
Channel Players\
Cinemaware\
CoreDesign\
CustomMade\
DariusZendeh\
Dave Lowe\
Dave Lowe New\
David Hanney\
Desire\
Digital Sonix And Chrome\
DigitalSoundStudio\
Dirk Bialluch\
Dynamic Synthesizer\
EarAche\
EMS (Editeur Musical Sequentiel)\
Fashion Tracker\
FredGray\
FutureComposer-BSI\
FuturePlayer\
GlueMon\
HowieDavies\
InStereo\
InStereo! 2.0\
JamCracker\
JankoMrsicFlogel\
JanneSalmijarviOptimizer\
JasonPage\
Jeroen Tel\
JesperOlsen\
JochenHippel-7V\
Kim Christensen\
KrisHatlelid\
LegglessMusicEditor\
Lionheart_Game\
MajorTom\
ManiacsOfNoise\
MarkII\
Mark_Cooksey\
Mark_Cooksey_Old\
MCMD\
Medley\
MIDI-Loriciel\
MikeDavies\
MMDC\
Mosh Packer\
MusicAssembler\
MusiclineEditor\
MusicMaker-8V\
Nick Pelling Packer\
NTSP-system\
onEscapee\
Paul Robotham\
Paul Tonge\
PaulShields\
PaulSummers\
PeterVerswyvelen\
ProfessionalSoundArtists\
PumaTracker\
Quartet PSG\
Quartet_ST\
RiffRaff\
RobHubbardOld\
SCUMM\
SeanConnolly\
SeanConran\
Silmarils\
SonicArranger\
SonicArranger-pc-all\
SonixMusicDriver\
SoundControl\
SoundFactory\
SoundImages\
SoundMaster\
SoundPlayer\
SoundProgrammingLanguage\
Special-FX\
Special-FX_ST\
SpeedyA1System\
SpeedySystem\
SteveBarrett\
SteveTurner (Jason Page Old)\
SUN-Tronic\
Synth (Synthesis)\
SynTracker\
TFMX\
TFMX-7V\
TFMX-Pro\
TFMX_ST\
TheMusicalEnlightenment\
ThomasHermann\
TimFollin\
TomyTracker\
Tronic (TronicTracker)\
VoodooSupremeSynthesizer\
WallyBeben\
YM-2149

### SUPPORTED PACKERS
None right now
