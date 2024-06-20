# WebUADE

Copyright (C) 2014 Juergen Wothke

This plugin is designed to work with my generic WebAudio ScriptProcessor music player (see separate project)
but the API exposed by the lib can be used in any JavaScipt program (it should look familiar to anyone 
that has ever done some sort of music player plugin). 

WebUADE is based on "uade-2.13" and this project still contains most of the original files (even if some of them are 
not used in this context). The various unused "frontend" subfolders and top level make-files have been completely removed. 

Some of the files were modified (see explanations below) but it should be easy to identify respective changes either 
by diff-ing against an original/unchanged uade-2.13 distribution or by searching for the "EMSCRIPTEN" #ifdefs which 
mark the changes. (The structure of the original code was preserved to ease comparison.)


## Howto build

You'll need Emscripten (http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). The make script 
is designed for use of emscripten version 1.37.29 (unless you want to create WebAssembly output, older versions might 
also still work).

The below instructions assume that the uade-2.13 project folder has been moved into the main emscripten 
installation folder (maybe not necessary) and that a command prompt has been opened within the 
project's "emscripten" sub-folder, and that the Emscripten environment vars have been previously 
set (run emsdk_env.bat).

The original "uade-2.13" makefiles must NOT be used because they might overwrite some of the manually performed changes. 
Instead the Web version is built using the makeEmscripten.bat that can be found in this folder. The script will compile 
directly into the "emscripten/htdocs" example web folder, were it will create the backend_uade.js library. The content of 
the "htdocs" can be tested by first copying into some document folder of a web server. You then need to manually create 
an 'uade' subfolder and copy the "players" and "amigasrc" sub-folders into it - also create a 'songs' folder here to 
hold the music files. A running example can be found here: http://www.wothke.ch/webuade

Known pitfall: When using FTP to upload the files to some server, make sure to use "binary mode" for the
files in the "uade/players" folder (otherwise some of them will get damaged)!


## Dependencies
The current version requires version 1.03 (older versions will not
support WebAssembly and remote file loading) of my https://github.com/wothke/webaudio-player.

## Background information

This is a short summary how the code was derived from the original "uade-2.13" (maybe this helps if ever the code 
needs to be migrated to a different version of UADE).

- I used Cygwin on WinXP to "./configure" and "make" the original "uade-2.13". This created the basic file layout that 
you'll find in this project.

- The original code uses a two process design where a "frontend" process interacts with a separate "emulator core" 
process. For the web version, the separate "frontend" is completely eliminated (and with it all the IPC based 
communication). The relevant APIs previously invoked via IPC are instead exposed such that they can be invoked 
directly from JavaScript.

- Some of the logic of the original frontend was added directly to the "emulator core" (e.g. Eagleplayer stuff).

- Any file loading is diverted to a central API (see standalonesupport.c which replaces the original unixsupport.c) - 
which is serviced from the HTML5/JavaScript side. Due to the asynchronous nature of Web based file loading, the 
respective API uses a slightly extended error handling model. Any code triggering a "file load" had to be extended 
to deal with the specific error scenario that a requested file is not yet available. To recover from such errors the 
JavaScript side player uses a "retry from scratch" approach that is triggered as soon as a loaded file becomes available.

- Whereas the original emulator permanently runs in a loop (see m68k_run_1()), the code was changed to now just run long
enough to produce the next batch of audio samples (see emu_compute_audio_samples()). The JavaScript side is in control - 
not the emulator.

- All the emulator APIs relevant for the JavaScript side are located in the new adapter.c. 

- The callback.js library provides JavaScript functionalities that are directly compilied into the emulator (mainly used 
for the interactions with the "native" JavaScript portions of the player). 

## Known Limitation

Not all Eagleplayer players will work: Some rely on the Amiga's "audio device" which is NOT available in UADE. 

 
## License

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version. This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA


