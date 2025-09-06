# BZR Player 2 (BZR2)

Audio player for **Windows** and **Linux** supporting a wide array of multi-platform **exotic** file
formats, written in **C++** and **Qt** with a sound engine based on **FMOD**.\
The first BZR version was released in 2008, the last 1.x in 2019: this is the beginning of the new 2.x version which is
coded pretty much from scratch.

## Download binaries

- [Releases & changelogs](https://github.com/aargirakis/BZRPlayer/releases)
- AUR package: [`bzr-player`](https://aur.archlinux.org/packages/bzr-player)
- [Old versions archive](https://github.com/aargirakis/BZRPlayer/tree/binaries_archive/binaries)

## How To Build

### Windows

**[MSYS2](https://www.msys2.org/)** with following packages is required:

`base-devel` `mingw-w64-ucrt-x86_64-cmake` `mingw-w64-ucrt-x86_64-qt6-base` `mingw-w64-ucrt-x86_64-qt6-svg`
`mingw-w64-ucrt-x86_64-qt-advanced-docking-system` `mingw-w64-ucrt-x86_64-SDL2` `mingw-w64-ucrt-x86_64-toolchain`
`openssl-devel`

From the MSYS2 **ucrt64.exe** command prompt go to the project sources dir (keep in mind Unix-style paths are
required), then start the configuration process executing:\
`cmake -B cmake-build -S . -DCMAKE_PREFIX_PATH=/ucrt64 -DCMAKE_BUILD_TYPE=`[`Debug`|`Release`]` -G Ninja`

To build the project execute:\
`ninja -C cmake-build`

As result of the building process, in the chosen CMake build directory the `output` directory will be populated with
binaries.\
If the **Release** build type is selected, along with `output` also `output_release` directory will be created,
containing the final archive release file

#### Build example

```
cd /c/BZRPlayer
cmake -B cmake-build -S . -DCMAKE_PREFIX_PATH=/ucrt64 -DCMAKE_BUILD_TYPE=Release -G Ninja &&
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

#### Windows installer

Although the **BZR2 installer for Windows** is scripted in **Nullsoft Scriptable Install System (NSIS)**, it can
be only compiled using **WSL2** or cross-compiled on Linux since it contains Linux specific code (mostly the bash script
for the XDG MIME types handling), also **MSYS2** it is currently not viable since the required **NSIS** plugins are
still missing.

**NSIS** (3.10 or newer) with following plugins (check AUR entries) is required:

- [AccessControl](https://nsis.sourceforge.io/AccessControl_plug-in) `nsis-accesscontrol-bin`
- [NsArray](https://nsis.sourceforge.io/Arrays_in_NSIS) `nsis-nsarray-bin`
- [NsProcess](https://nsis.sourceforge.io/NsProcess_plugin) `nsis-nsprocess-bin`
- [Registry](https://nsis.sourceforge.io/Registry_plug-in) `nsis-registry-bin`

In order to build the Windows installer put the target binaries in `src/inst/nsis/bin` then enter `src/inst/nsis`
directory and execute: `makensis -DVERSION="<any_version>" bzr2_setup.nsi`\
As result of the building process `bzr-player-<any_version>-win64.exe` will be generated in the same directory.

### Linux

In order to build BZR2 following packages are required:

- On **Arch-based** distros:\
  `base-devel` `cmake` `dos2unix` `libglvnd` `ninja` `patchutils` `qt6-base` `qt-advanced-docking-system`
  `qt6-declarative` `qt6-svg` `sdl2-compat` `vulkan-headers`


- On **Debian-based** distros:\
  `build-essential` `cmake` `dos2unix` `libglvnd0` `libsdl2-dev` `libvulkan-dev` `ninja-build` `patchutils`
  `qt6-base-dev` `qt6-base-private-dev` `qt6-declarative-dev` `qt6-svg-dev`
  [libqt-advanced-docking-system4.4.0-dev](https://github.com/aargirakis/BZRPlayer/releases/latest/download/libqt-advanced-docking-system-dev4.4.0_4.4.0-0_amd64.deb)


- On **Fedora-based** distros:\
  `development-tools` (group), `cmake` `dos2unix` `ninja-build` `qt6-qtbase-devel` `qt6-qtsvg-devel` `sdl2-compat-devel`
  `vulkan-headers` `which`
  [libqtadvanceddockingsystem-qt6-devel](https://github.com/aargirakis/BZRPlayer/releases/latest/download/libqtadvanceddockingsystem-qt6-devel-4.4.0-0.x86_64.rpm)

---

Go to the project sources dir then start the configuration process executing:\
`cmake -B cmake-build -S . -DCMAKE_PREFIX_PATH=/usr -DCMAKE_BUILD_TYPE=`[`Debug`|`Release`]` -G Ninja`

To build the project execute:\
`ninja -C cmake-build`

#### Build example

```
cd ~/bzr-player &&
cmake -B cmake-build -S . -DCMAKE_PREFIX_PATH=/usr -DCMAKE_BUILD_TYPE=Debug -G Ninja &&
ninja -C cmake-build 
```

#### Runtime dependencies

For running BZR2 following packages are required:

- On **Arch-based** distros:\
  `qt6-base` `qt6-svg` `qt-advanced-docking-system`


- On **Debian-based** distros:\
  `libqt6core6` `libqt6network6` `libqt6openglwidgets6` `libqt6svg6` `libqt6xml6`
  [libqt-advanced-docking-system4.4.0](https://github.com/aargirakis/BZRPlayer/releases/latest/download/libqt-advanced-docking-system4.4.0_4.4.0-0_amd64.deb)


- On **Fedora-based** distros:\
  `qt6-qtbase` `qt6-qtsvg`
  [libqtadvanceddockingsystem-qt6](https://github.com/aargirakis/BZRPlayer/releases/latest/download/libqtadvanceddockingsystem-qt6-4.4.0-0.x86_64.rpm)

#### Generated binaries

On Linux, as result of the building process, in the chosen CMake build directory the `output` directory will be
populated with binaries.\
If the **Debug** build type is selected BZR2 will use development paths instead of system ones: this means all paths
will refer to the `output` directory (including the user settings and the executable's RPATH) ensuring a complete
isolation from any other (real) BZR2 installation (ideal for development purposes).\
If **Release** build type is selected then the system paths will be used (this also means generated binaries will work
only if they are installed in the correct system paths)

### Offline mode

By default, the CMake configuration stage will download all needed libraries and files. Add `-DOFFLINE_MODE=1` to CMake
command for switching to offline mode.\
Offline mode doesn't guarantee that the build will include the latest versions of the files with unmanaged version

----

## Useful links:

- [BZR2 website](https://bzrplayer.blazer.nu)
- [Patreon](https://www.patreon.com/bzrplayer)
- [Discord](https://discord.gg/feEBce8cFe)

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
