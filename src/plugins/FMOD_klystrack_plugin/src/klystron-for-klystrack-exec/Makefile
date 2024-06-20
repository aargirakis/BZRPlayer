TARGET=engine
VPATH=src:src
ECHO = echo
CFG = debug
REV = cp -f
MACHINE =

include common.mk

util_SRC = $(notdir ${wildcard src/util/*.c})
util_DEP = $(patsubst %.c, deps/util_$(CFG)_%.d, ${util_SRC})
util_OBJ = $(patsubst %.c, objs.$(CFG)/util_%.o, ${util_SRC})

snd_SRC = $(notdir ${wildcard src/snd/*.c})
snd_DEP = $(patsubst %.c, deps/snd_$(CFG)_%.d, ${snd_SRC})
snd_OBJ = $(patsubst %.c, objs.$(CFG)/snd_%.o, ${snd_SRC})

gfx_SRC = $(notdir ${wildcard src/gfx/*.c})
gfx_DEP = $(patsubst %.c, deps/gfx_$(CFG)_%.d, ${gfx_SRC})
gfx_OBJ = $(patsubst %.c, objs.$(CFG)/gfx_%.o, ${gfx_SRC})

gui_SRC = $(notdir ${wildcard src/gui/*.c})
gui_DEP = $(patsubst %.c, deps/gui_$(CFG)_%.d, ${gui_SRC})
gui_OBJ = $(patsubst %.c, objs.$(CFG)/gui_%.o, ${gui_SRC}) $(util_OBJ) $(gfx_OBJ)

lib_SRC = $(notdir ${wildcard src/lib/*.c})
lib_DEP = $(patsubst %.c, deps/lib_$(CFG)_%.d, ${lib_SRC})
lib_OBJ = $(patsubst %.c, objs.$(CFG)/lib_%.o, ${lib_SRC})

CC = gcc -shared -std=gnu99 -Wno-strict-aliasing
CDEP = gcc -E -std=gnu99

ifndef CFLAGS
CFLAGS = $(MACHINE) -ftree-vectorize
endif

INCLUDEFLAGS= -I ../Common -I src $(SDLFLAGS) -I src/gfx -I src/snd -I src/util -I src/gui $(EXTFLAGS)

# Separate compile options per configuration
ifeq ($(CFG),debug)
	CFLAGS += -O3 -g -Wall ${INCLUDEFLAGS} -DDEBUG -fno-inline
else
	ifeq ($(CFG),profile)
		CFLAGS += -O3 -pg -Wall ${INCLUDEFLAGS}
	else
		ifeq ($(CFG),release)
			CFLAGS += -O3 -Wall ${INCLUDEFLAGS} -s
		else
			ifeq ($(CFG),size)
				CFLAGS += -Os -Wall ${INCLUDEFLAGS} -s -ffast-math -fomit-frame-pointer -DREDUCESIZE
			else
				@$(ECHO) "Invalid configuration "$(CFG)" specified."
				@$(ECHO) "You must specify a configuration when "
				@$(ECHO) "running make, e.g. make CFG=debug"
				@$(ECHO) "Possible choices for configuration are "
				@$(ECHO) "'release', 'profile', 'size' and 'debug'"
				@exit 1
			endif
		endif
	endif
endif

# A common link flag for all configurations
LDFLAGS =

.PHONY: tools all build

build: Makefile
	$(Q)echo '#ifndef KLYSTRON_VERSION_H' > ./src/version.h
	$(Q)echo '#define KLYSTRON_VERSION_H' >> ./src/version.h
	$(Q)echo '#define KLYSTRON_REVISION "' | tr -d '\n' >> ./src/version.h
	$(Q)date +"%Y%m%d" | tr -d '\n' >> ./src/version.h
	$(Q)echo '"' >> ./src/version.h
	$(Q)echo '#define KLYSTRON_VERSION_STRING "klystron " KLYSTRON_REVISION' >> ./src/version.h
	$(Q)echo '#endif' >> ./src/version.h
	make all CFG=$(CFG)

all: bin.$(CFG)/lib${TARGET}_snd.a bin.$(CFG)/lib${TARGET}_gfx.a bin.$(CFG)/lib${TARGET}_util.a bin.$(CFG)/lib${TARGET}_gui.a tools

ksnd: bin.$(CFG)/libksndstatic.a bin.$(CFG)/ksnd.dll

ifdef COMSPEC
tools: tools/bin/makebundle.exe tools/bin/editor.exe
else
tools: tools/bin/makebundle.exe
endif

inform:
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/lib${TARGET}_snd.a: ${snd_OBJ} | inform
	$(MSG) "Linking "$(TARGET)"..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)ar rcs $@ $^

bin.$(CFG)/lib${TARGET}_gfx.a: ${gfx_OBJ} | inform
	$(MSG) "Linking "$(TARGET)"..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)ar rcs $@ $^

bin.$(CFG)/lib${TARGET}_util.a: ${util_OBJ} | inform
	$(MSG) "Linking "$(TARGET)"..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)ar rcs $@ $^

bin.$(CFG)/lib${TARGET}_gui.a: ${gui_OBJ} | inform
	$(MSG) "Linking "$(TARGET)"..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)ar rcs $@ $^

bin.$(CFG)/libksndstatic.a: objs.$(CFG)/lib_ksnd.o ${snd_OBJ} | inform
	$(MSG) "Linking "$(TARGET)"..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)ar rcs $@ $^
ifdef COMSPEC
	$(MSG) "Building ksndstatic.lib..."
	@-lib /OUT:bin.$(CFG)/ksndstatic.lib $^
endif

bin.$(CFG)/ksnd.dll: objs.$(CFG)/lib_ksnd.o ${snd_OBJ} src/lib/ksnd.def | inform
	$(MSG) "Linking ksnd.dll..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)$(CC) -shared -o $@ objs.$(CFG)/lib_ksnd.o src/lib/ksnd.def ${snd_OBJ} $(CFLAGS) $(INCLUDEFLAGS) -DDLLEXPORT -Wl,--out-implib,bin.$(CFG)/libksnd.a
ifdef COMSPEC
	$(MSG) "Building ksnd.lib..."
	@-lib /DEF:src/lib/ksnd.def /OUT:bin.$(CFG)/ksnd.lib
endif

objs.$(CFG)/snd_%.o: snd/%.c
	$(MSG) "Compiling "$(notdir $<)"..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

objs.$(CFG)/gfx_%.o: gfx/%.c
	$(MSG) "Compiling "$(notdir $<)"..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

objs.$(CFG)/util_%.o: util/%.c
	$(MSG) "Compiling "$(notdir $<)"..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

objs.$(CFG)/gui_%.o: gui/%.c
	$(MSG) "Compiling "$(notdir $<)"..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

objs.$(CFG)/lib_%.o: lib/%.c
	$(MSG) "Compiling "$(notdir $<)"..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

deps/snd_$(CFG)_%.d: snd/%.c
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $<"
	$(Q)set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/snd_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/gfx_$(CFG)_%.d: gfx/%.c
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $<"
	$(Q)set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/gfx_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/util_$(CFG)_%.d: util/%.c
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $<"
	$(Q)set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/util_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/gui_$(CFG)_%.d: gui/%.c
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $<"
	$(Q)set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/gui_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/lib_$(CFG)_%.d: lib/%.c
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $<"
	$(Q)set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/gui_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	$(Q)rm -rf deps objs.release objs.debug objs.profile objs.size bin.release bin.debug bin.profile bin.size

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first
# invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${snd_DEP}
-include ${gfx_DEP}
-include ${util_DEP}
-include ${gui_DEP}
-include ${lib_DEP}
endif

tools/bin/makebundle.exe: tools/makebundle/*.c
	make -C tools/makebundle

ifdef COMSPEC
tools/bin/editor.exe: tools/editor/src/*
	make -C tools/editor
	cp tools/editor/bin.$(CFG)/editor.exe tools/bin/editor.exe
endif
