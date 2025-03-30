# BZR Player 2 (BZR2)

Audio player for **Windows** and **Linux** (using **Wine**) supporting a wide array of multi-platform **exotic** file
formats, written in **C++** and **Qt5** with a sound engine based on **FMOD**.\
The first BZR version was released in 2008, the last 1.x in 2019: this is the beginning of the new 2.x version which is
coded pretty much from scratch.

## Download binaries

- Windows & Linux online installers: https://github.com/aargirakis/BZRPlayer/blob/main/src/inst
- AUR package: `bzr-player` https://aur.archlinux.org/packages/bzr-player

## How To Build

### Windows

**[MSYS2](https://www.msys2.org/)** with following packages is required:

`make` `mingw-w64-ucrt-x86_64-cmake` `mingw-w64-ucrt-x86_64-qt5-base` `mingw-w64-ucrt-x86_64-qt5-svg`
`mingw-w64-ucrt-x86_64-SDL2` `mingw-w64-ucrt-x86_64-toolchain` `openssl-devel` `patch`

From the MSYS2 **ucrt64.exe** command prompt go to the project sources dir (take in mind Unix-style paths are
required), then start the configuration process executing:\
`cmake -S . -B cmake-build -DCMAKE_PREFIX_PATH=/ucrt64 -DCMAKE_BUILD_TYPE=`[`Debug`|`Release`]` -G Ninja`

To build the project execute:\
`ninja -C cmake-build`

As result of the building process, in the chosen CMake build directory the `output` directory will be populated with
binaries.\
If the **Release** build type is selected, along with `output` also `output_release` directory will be created,
containing the final archive release file

#### build example

```
cd /c/BZRPlayer
cmake -S . -B cmake-build -DCMAKE_PREFIX_PATH=/ucrt64 -DCMAKE_BUILD_TYPE=Release -G Ninja &&
ninja -C cmake-build 
```

#### IDE setup

These are the settings for any IDE that supports CMake:

- set the toolchain to **<MSYS2_dir>\ucrt64**\
  (e.g. `C:\msys64\ucrt64`)


- set the CMake command with following flags:\
  **-DCMAKE_PREFIX_PATH="<MSYS2_dir>/ucrt64" -G "Ninja"**\
  (e.g. `-DCMAKE_PREFIX_PATH="c:/msys64/ucrt64" -G "Ninja"`)


- set additional environment variables **MSYSTEM=UCRT64** and **PATH=<MSYS2_dir>/usr/bin**\
  (e.g. `MSYSTEM=UCRT64;PATH=c:/msys64/usr/bin`)


- set the CMake application runner to build **All targets** with `app` as executable


- (optional) set CMake executable to **<MSYS2_dir>\ucrt64\bin\cmake.exe**\
  (e.g. `C:\msys64\ucrt64\bin\cmake.exe`)

### Linux (cross-compilation)

Dockerized cross-compilation toolchain is provided, just execute `run.sh` from the **docker** directory with following
flags:

- `CONFIG=1` for running the CMake configuration stage (with **Debug** build type, eventually setting
  `BUILD_TYPE=Release` if needed)
- `BUILD=1` for building the project
- `RUN_BZR2=1` for running built BZR2 (**Wine** is required)

### Offline mode

By default, the CMake configuration stage will download all needed libraries and files. Add `-DOFFLINE_MODE=1` to CMake
command (or `OFFLINE_MODE=1` to `run.sh`) for switching to offline mode.\
Offline mode doesn't guarantee that the build will include the latest versions of the files with unmanaged version

### Windows online installer

Although the **BZR2 online installer for Windows** is scripted in **Nullsoft Scriptable Install System (NSIS)**, it can
be only compiled using **WSL2** or cross-compiled on Linux since it contains Linux specific code (mostly the bash script
for the XDG MIME types handling), also **MSYS2** it is currently not viable since the required **NSIS** plugins are
still missing.

**NSIS** (3.10 or newer) with following plugins (check AUR entries) is required:

- [AccessControl](https://nsis.sourceforge.io/AccessControl_plug-in) `nsis-accesscontrol-bin`
- [Inetc](https://nsis.sourceforge.io/Inetc_plug-in) `nsis-inetc-bin`
- [NsArray](https://nsis.sourceforge.io/Arrays_in_NSIS) `nsis-nsarray-bin`
- [Nsisunz](https://nsis.sourceforge.io/Nsisunz_plug-in) `nsis-nsisunz-bin`
- [NsJSON](https://nsis.sourceforge.io/NsJSON_plug-in) `nsis-nsjson-bin`
- [NsProcess](https://nsis.sourceforge.io/NsProcess_plugin) `nsis-nsprocess-bin`
- [NsRichEdit](https://nsis.sourceforge.io/NsRichEdit_plug-in) `nsis-nsrichedit-bin`
- [Registry](https://nsis.sourceforge.io/Registry_plug-in) `nsis-registry-bin`

In order to build the Windows installer enter `src/inst/nsis` directory then execute: `makensis bzr2_setup.nsi`\
As result of the building process (the Wine compatible) `bzr2_setup.exe` will be generated in the same directory.\
Since it is a self-updating installer, the latest installer version check is done at runtime (based on
`bzr2_setup.exe_latest` file content generated at compile-time).\
Useful flags for dev/testing purposes:

- `skipInstallerUpdate2` skip latest installer's version check/download: `bzr2_setup.exe /skipInstallerUpdate2`
- `localReleaseArchivePath` install BZR2 from a local release archive:
  `bzr2_setup.exe /localBinaryPath="<path_to_release_archive_file>"`

----

#### Useful links:

- [BZR2 website](https://bzrplayer.blazer.nu)
- [Patreon](https://www.patreon.com/bzrplayer)
- [Discord](https://discord.com/channels/893241318120239124/893241440174477382)

![bzr2 0 27](https://user-images.githubusercontent.com/10993634/201359947-2633341d-9ff6-4a59-bb9e-ce1794df9cba.png)

----

## SUPPORTED FORMATS (#TODO wrong & incomplete)

### Using Libsidplayfp

Commodore 64 SID, PSID<br/>

### Using Audio File Library

Amiga IFF/8SVX<br/>
Audio Visual Research<br/>
Berkeley/IRCAM/CARL<br/>
Compressed AIFF<br/>
NeXT .snd<br/>
NIST SPHERE<br/>
SampleVision<br/>
Sun .au<br/>

### Using ASAP

Atari systems using POKEY sound chip<br/>
Chaos Music Composer<br/>
Chaos Music Composer -3-4-<br/>
Chaos Music Composer -Rzog-<br/>
Delta Music Composer<br/>
DoublePlay Chaos Music Composer<br/>
Music ProTracker<br/>
Raster Music Tracker<br/>
Stereo Double Chaos Music Composer<br/>
Theta Music Composer 1.x 4-channel<br/>
Theta Music Composer 1.x 8-channel<br/>
Theta Music Composer 2.x<br/>

### Using Game Music Emu

AY - ZX Spectrum, Amstrad CPC<br/>
GBS - Nintendo Game Boyr<br/>
GYM - Sega Genesis, Mega Drive<br/>
HES - NEC TurboGrafx-16, PC Engine<br/>
KSS - MSX Home Computer, other Z80 systems<br/>
NSF - NES Sound Format<br/>
NSFE - NES Sound Format Extended<br/>
SPC - Super Nintendo, Super Famicom<br/>
VGM - Video Game Music File<br/>
VGZ - Compressed Video Game Music File<br/>
RSN - RAR archive with SPC songs<br/>

### Using HivelyTracker

AHX<br/>
HivelyTracker<br/>

### Using Mamiya's m_s98.kpi

S98

### Using Ken's Digital Music

Ken's Digital Music

### Using libpac

SBStudio PAC

### Using LibV2

Farbrausch V2M

### Using Organya

Organya<br/>

### Using UADE

ActionAmics<br/>
Activision Pro (MartinWalker)<br/>
Alcatraz_Packer<br/>
AM-Composer<br/>
Anders Øland<br/>
Andrew Parton<br/>
Art And Magic<br/>
ArtOfNoise-4V<br/>
ArtOfNoise-8V<br/>
Ashley Hogg<br/>
BeathovenSynthesizer<br/>
Ben Daglish SID<br/>
BladePacker<br/>
Channel Players<br/>
Cinemaware<br/>
CoreDesign<br/>
CustomMade<br/>
DariusZendeh<br/>
Dave Lowe<br/>
Dave Lowe New<br/>
David Hanney<br/>
Desire<br/>
Digital Sonix And Chrome<br/>
DigitalSoundStudio<br/>
Dirk Bialluch<br/>
Dynamic Synthesizer<br/>
EarAche<br/>
EMS (Editeur Musical Sequentiel)<br/>
Fashion Tracker<br/>
FredGray<br/>
FutureComposer-BSI<br/>
FuturePlayer<br/>
GlueMon<br/>
HowieDavies<br/>
InStereo<br/>
InStereo! 2.0<br/>
JamCracker<br/>
JankoMrsicFlogel<br/>
JanneSalmijarviOptimizer<br/>
JasonPage<br/>
Jeroen Tel<br/>
JesperOlsen<br/>
JochenHippel-7V<br/>
Kim Christensen<br/>
KrisHatlelid<br/>
LegglessMusicEditor<br/>
Lionheart_Game<br/>
MajorTom<br/>
ManiacsOfNoise<br/>
MarkII<br/>
Mark_Cooksey<br/>
Mark_Cooksey_Old<br/>
MCMD<br/>
Medley<br/>
MIDI-Loriciel<br/>
MikeDavies<br/>
MMDC<br/>
Mosh Packer<br/>
MusicAssembler<br/>
MusiclineEditor<br/>
MusicMaker-8V<br/>
Nick Pelling Packer<br/>
NTSP-system<br/>
onEscapee<br/>
Paul Robotham<br/>
Paul Tonge<br/>
PaulShields<br/>
PaulSummers<br/>
PeterVerswyvelen<br/>
ProfessionalSoundArtists<br/>
PumaTracker<br/>
Quartet PSG<br/>
Quartet_ST<br/>
RiffRaff<br/>
RobHubbardOld<br/>
SCUMM<br/>
SeanConnolly<br/>
SeanConran<br/>
Silmarils<br/>
SonicArranger<br/>
SonicArranger-pc-all<br/>
SonixMusicDriver<br/>
SoundControl<br/>
SoundFactory<br/>
SoundImages<br/>
SoundMaster<br/>
SoundPlayer<br/>
SoundProgrammingLanguage<br/>
Special-FX<br/>
Special-FX_ST<br/>
SpeedyA1System<br/>
SpeedySystem<br/>
SteveBarrett<br/>
SteveTurner (Jason Page Old)<br/>
SUN-Tronic<br/>
Synth (Synthesis)<br/>
SynTracker<br/>
TFMX<br/>
TFMX-7V<br/>
TFMX-Pro<br/>
TFMX_ST<br/>
TheMusicalEnlightenment<br/>
ThomasHermann<br/>
TimFollin<br/>
TomyTracker<br/>
Tronic (TronicTracker)<br/>
VoodooSupremeSynthesizer<br/>
WallyBeben<br/>
YM-2149<br/>

### SUPPORTED PACKERS

None right now
