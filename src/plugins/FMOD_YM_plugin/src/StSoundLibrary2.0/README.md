StSound Library 2.0
===================

[![Build Status](https://travis-ci.org/cpcsdk/libstsound.svg?branch=master)](https://travis-ci.org/cpcsdk/libstsound)


by the CPCSDK team

What you will find here is a fork of StSoundLibrary as released by
Leonard/Oxygene. We needed a version of this library with some changes for
better Amstrad CPC support, that is, emulating an AY-3-8912 chip instead of the
YM2149 (there are some slight differences), with stereo support. Our goal was
to use it in an emulator, so we had to change the API. We also wanted to create
a more practical build system for it.

This version of StSound use a different API, some change into source code are
probably need. Stereo are know working fine and a system of profile are create
for change emulation parameters (MasterClock, Volume Table and Mixing). And a
system of modular filter had been added.

StSoundLibrary 2.0 is very different from the original. It is far more accurate
as we use volume curves gathered from measurements on a real machine and
finetuning done with highly trained ears. As of today, it is the most accurate
AY3 emulator available.
