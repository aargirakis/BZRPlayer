MAKE = make
CC   = gcc -mcrt=newlib
CP	 = copy
RM   = delete

# Change these as required
OPTIMIZE = -O3
DEBUG = 
CFLAGS = $(OPTIMIZE) $(DEBUG) -I. -Iinclude -Iconfig68 \
	-DHAVE_CONFIG_CONFIG68_H=1 #-DDEBUG

# Flags passed to gcc during linking
LINK = $(OPTIMIZE) $(DEBUG)

# Additional linker libraries
LIBS = -lz

# Name of the "thing" to build
TARGET  = TN_SC68.tnplug
VERSION = 1
SC68BIN = TN_SC68.bin

# Source code files used in this project
# Add any additional files to this line

SRCS = init.c initplayer.c announceplayer.c openplayer.c decodeframeplayer.c donotify.c

SC68_SRCS = sc68/sc68bin.c \
	api68/api68.c api68/conf68.c api68/mixer68.c \
	emu68/table68.c emu68/mem68.c emu68/lineF_68.c emu68/lineE_68.c emu68/lineD_68.c \
	emu68/lineC_68.c emu68/lineB_68.c emu68/lineA_68.c emu68/line9_68.c emu68/line8_68.c \
	emu68/line7_68.c emu68/line6_68.c emu68/line5_68.c emu68/line4_68.c emu68/line3_68.c \
	emu68/line2_68.c emu68/line1_68.c emu68/line0_68.c emu68/ioplug68.c emu68/inst68.c \
	emu68/getea68.c emu68/error68.c emu68/emu68.c emu68/cc68.c \
	file68/url68.c file68/string68.c file68/rsc68.c file68/registry68.c file68/istream68_z.c \
	file68/istream68_null.c file68/istream68_mem.c file68/istream68_file.c file68/istream68_fd.c \
	file68/istream68_curl.c file68/istream68.c file68/init68.c file68/ice68.c file68/gzip68.c \
	file68/file68.c file68/error68.c file68/endian68.c file68/debugmsg68.c file68/alloc68.c \
	io68/ym_io.c io68/ymemul.c io68/shifter_io.c io68/paula_io.c io68/paulaemul.c io68/mw_io.c \
	io68/mwemul.c io68/mfp_io.c io68/mfpemul.c \
	unice68/unice68_native.c

# -------------------------------------------------------------
# Nothing should need changing below this line

OBJS = $(SRCS:.c=.o)

SC68_OBJS = $(SC68_SRCS:.c=.o)

all: $(TARGET) data/$(SC68BIN)

# Rules for building
$(TARGET): $(OBJS)
	$(CC) $(LINK) -nostartfiles -o $@ $^ $(LIBS)

init.o announceplayer.o: $(TARGET)_rev.h

data/$(SC68BIN): $(SC68_OBJS)
	$(CC) $(LINK) -nostartfiles -o $@ $^ $(LIBS)

sc68/sc68bin.o: $(SC68BIN)_rev.h

.PHONY: install
install: $(TARGET) data/$(SC68BIN)
	$(CP) $(TARGET) Apps:TuneNet/Plugins/
	$(CP) data/$(SC68BIN) Apps:TuneNet/Plugins/data/

.PHONY: clean
clean: distclean
	$(RM) $(TARGET) data/$(SC68BIN)

.PHONY: distclean
distclean:
	$(RM) $(OBJS) $(SC68_OBJS)

.PHONY: revision
revision:
	bumprev $(VERSION) $(TARGET)
	bumprev $(VERSION) $(SC68BIN)
