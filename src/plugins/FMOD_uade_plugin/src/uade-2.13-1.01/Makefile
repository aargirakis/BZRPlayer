# Top-level Makefile for UADE

MAKE = /usr/bin/make

BINDIR = /usr/local/bin
DATADIR = /usr/local/share/uade2
DOCDIR = {DOCDIR}
MANDIR = /usr/local/share/man/man1
LIBDIR = /usr/local/lib/uade2

UADECORENAME=uadecore.exe
UADE123NAME=uade123.exe

COMPILED_COMPONENTS = uadecore uade123   
INSTALLED_COMPONENTS = uadecoreinstall uade123install   

all:	src/include/uadeconfig.h $(COMPILED_COMPONENTS)

uadecore:	
	$(MAKE) -C src

uadecoreinstall:	
	$(MAKE) -C src install

uade123:	
	$(MAKE) -C src/frontends/uade123

uade123install:	
	$(MAKE) -C src/frontends/uade123 install

xmmsplugin:	
	$(MAKE) -C src/frontends/xmms

xmmsplugininstall:	
	$(MAKE) -C src/frontends/xmms install

audaciousplugin:	
	$(MAKE) -C src/frontends/audacious

audaciousplugininstall:	
	$(MAKE) -C src/frontends/audacious install

uadefs:	
	$(MAKE) -C src/frontends/uadefs

uadefsinstall:	
	$(MAKE) -C src/frontends/uadefs install

src/include/uadeconfig.h:	
	@echo ""
	@echo "Run ./configure first!"
	@echo ""
	@false

soundcheck:	
	@ echo ""
	@ echo "### UADE should be playing now 'AHX.Cruisin'"
	@ echo ""
	src/frontends/uade123/$(UADE123NAME) --basedir=. -S amigasrc/score/score -P players/AbyssHighestExperience songs/AHX.Cruisin -u src/uadecore

generalinstall:
	mkdir -p "$(DATADIR)"
	test -e "$(DATADIR)/uade.conf" || cp -f uade.conf "$(DATADIR)/"
	cp -f amigasrc/score/score uaerc eagleplayer.conf "$(DATADIR)/"
	cp -rf players "$(DATADIR)/" || true
	chmod -R og+rX "$(DATADIR)"

	mkdir -p "$(BINDIR)"
	install src/frontends/mod2ogg/mod2ogg2.sh "$(BINDIR)/"

	mkdir -p "$(MANDIR)"
	cp doc/uade123.1 "$(MANDIR)"/
	chmod og+r "$(MANDIR)"/uade123.1

install:	generalinstall $(INSTALLED_COMPONENTS)
	@echo

uninstall:	
# Don't remove everything
	for name in players score ; do rm -rf -- "$(DATADIR)/$$name" ; done
	rm -f "$(MANDIR)"/uade123.1
	rm -f "$(LIBDIR)/$(UADECORENAME)"

feclean:	
	$(MAKE) -C src/frontends/uade123 clean
	$(MAKE) -C src/frontends/xmms clean
	$(MAKE) -C src/frontends/audacious clean
	$(MAKE) -C src/frontends/uadefs clean

clean:	
	$(MAKE) -C src clean
	$(MAKE) feclean
