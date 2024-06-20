ifneq ($(findstring $(platform),windows mingw),)
support_waveout = 1
#support_aylpt_dlportio = 1
support_directsound = 1
#support_sdl = 1
support_mp3 = 1
support_ogg = 1
support_flac = 1
support_curl = 1
else ifneq ($(findstring $(platform),linux),)
support_oss = 1
support_alsa = 1
#support_sdl = 1
support_mp3 = 1
support_ogg = 1
support_flac = 1
support_curl = 1
else ifneq ($(findstring $(platform),dingux),)
support_oss = 1
#support_sdl = 1
else ifneq ($(findstring $(platform),macos),)
support_sdl = 1
support_curl = 1
else ifeq ($(platform),android)
#no features
else
$(warning Unknown platform)
endif
