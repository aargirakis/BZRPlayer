prefix := /usr/local
srcdir := $(dir $(lastword $(MAKEFILE_LIST)))
bindir = $(prefix)/bin
libdir = $(prefix)/lib
CC = gcc
CFLAGS = -O2 -Wall
CPPFLAGS =
LDFLAGS = -s
AR = ar
ARFLAGS = rc
DO_CC = $(DO)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(if $(filter %.so,$@),-shared -fPIC) $(INCLUDEOPTS) $(filter %.c,$^) $(LDFLAGS)
DO_AR = $(DO)$(AR) $(ARFLAGS) $@ $^
CITO = $(DO)cito -o $@ $(patsubst %,-I %,$(sort $(dir $(filter-out %.ci,$^)))) $(filter %.ci,$^)
INSTALL = install
INSTALL_PROGRAM = mkdir -p $(DESTDIR)$(2) && $(INSTALL) $(1) $(DESTDIR)$(2)/$(or $(3),$(1))
INSTALL_DATA = mkdir -p $(DESTDIR)$(2) && $(INSTALL) -m 644 $(1) $(DESTDIR)$(2)/$(1)
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`
SEVENZIP = 7z a -mx=9 -bd -bso0
MAKEZIP = $(DO)$(RM) $@ && $(SEVENZIP) -tzip $@ $(patsubst %,./%,$(filter-out win32/signed,$^)) # "./" makes 7z don't store paths in the archive
COPY = $(DO)cp $< $@
XASM = $(DO)xasm -q -o $@ $<
OSX_CC = $(DO)cc -O2 -Wall -o $@ -undefined dynamic_lookup -mmacosx-version-min=10.6 -arch x86_64 -arch arm64 $(INCLUDEOPTS) $(filter %.c,$^)

# no user-configurable paths below this line

MAKEFLAGS = -r
ifdef V
DO = mkdir -p $(@D) && 
else
DO = @echo $@ && mkdir -p $(@D) && 
endif
src = $(addprefix $(srcdir),$(1:.[ch]=.c) $(patsubst %.[ch],%.h,$(filter %.[ch],$(1))))
INCLUDEOPTS = $(patsubst %/,-I%,$(sort $(dir $(filter %.h,$^))))
CLEAN :=
CLEANDIR :=

all: asapconv libasap.a
.PHONY: all

install: install-asapconv install-lib
.PHONY: install

include $(srcdir)6502/6502.mk
include $(srcdir)www/www.mk
include $(srcdir)release/release.mk
include $(srcdir)test/test.mk

# asapconv

asapconv: $(call src,asapconv.c asap.[ch])
	$(DO_CC)
CLEAN += asapconv

install-asapconv: asapconv
	$(call INSTALL_PROGRAM,asapconv,$(bindir))
.PHONY: install-asapconv

uninstall-asapconv:
	$(RM) $(DESTDIR)$(bindir)/asapconv
.PHONY: uninstall-asapconv

# lib

lib: libasap.a
.PHONY: lib

libasap.a: asap.o
	$(DO_AR)
CLEAN += libasap.a

asap.o: $(call src,asap.[ch])
	$(DO_CC) -c
CLEAN += asap.o

install-lib: libasap.a $(srcdir)asap.h
	$(call INSTALL_DATA,$(srcdir)asap.h,$(prefix)/include)
	$(call INSTALL_DATA,libasap.a,$(libdir))
.PHONY: install-lib

uninstall-lib:
	$(RM) $(DESTDIR)$(prefix)/include/asap.h $(DESTDIR)$(libdir)/libasap.a
.PHONY: uninstall-lib

# SDL

asap-sdl: $(call src,asap-sdl.c asap.[ch])
	$(DO_CC) $(SDL_CFLAGS) $(SDL_LIBS)
CLEAN += asap-sdl

install-sdl: asap-sdl
	$(call INSTALL_PROGRAM,asap-sdl,$(bindir))
.PHONY: install-sdl

uninstall-sdl:
	$(RM) $(DESTDIR)$(bindir)/asap-sdl
.PHONY: uninstall-sdl

# asapscan

asapscan: $(srcdir)asapscan.c asap-asapscan.h
	$(DO_CC)
CLEAN += asapscan asapscan.exe

asap-asapscan.h: $(call src,asap.ci asap6502.ci asapinfo.ci cpu6502.ci pokey.ci) $(ASM6502_PLAYERS_OBX) | asap-asapscan.c

asap-asapscan.c: $(call src,asap.ci asap6502.ci asapinfo.ci cpu6502.ci pokey.ci) $(ASM6502_PLAYERS_OBX)
	$(CITO) -D ASAPSCAN
CLEAN += asap-asapscan.c asap-asapscan.h

# asap.[ch]

$(srcdir)asap.h: $(call src,asap.ci asap6502.ci asapinfo.ci asapwriter.ci cpu6502.ci flashpack.ci pokey.ci) $(ASM6502_OBX) | $(srcdir)asap.c

$(srcdir)asap.c: $(call src,asap.ci asap6502.ci asapinfo.ci asapwriter.ci cpu6502.ci flashpack.ci pokey.ci) $(ASM6502_OBX)
	$(CITO) -D C

# aatr.[ch]

$(srcdir)aatr.h: $(srcdir)aatr.ci | $(srcdir)aatr.c

$(srcdir)aatr.c: $(srcdir)aatr.ci
	$(CITO)

clean:
	$(RM) $(CLEAN)
	$(RM) -r $(CLEANDIR)
.PHONY: clean

.DELETE_ON_ERROR:

include $(srcdir)moc/moc.mk
include $(srcdir)vlc/vlc.mk
include $(srcdir)xmms/xmms.mk
include $(srcdir)xmms2/xmms2.mk

include $(srcdir)csharp/csharp.mk
include $(srcdir)java/java.mk
include $(srcdir)javascript/javascript.mk
include $(srcdir)opencl/opencl.mk
include $(srcdir)python/python.mk
include $(srcdir)swift/swift.mk
include $(srcdir)win32/win32.mk
