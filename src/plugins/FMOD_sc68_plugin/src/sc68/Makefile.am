SUBDIRS = @SUBDIRS68@

bin_SCRIPTS = sc68-config

EXTRA_DIST =\
 doc/Makefile.in\
 doc/devdoc.hin\
 doc/doxygeneric.sh
# sc68.sln

my_install_headers =\
 ./api68/api68.h\
 ./api68/conf68.h\
 ./api68/mixer68.h\
 ./desa68/desa68.h\
 ./emu68/cc68.h\
 ./emu68/emu68.h\
 ./emu68/error68.h\
 ./emu68/excep68.h\
 ./emu68/getea68.h\
 ./emu68/inst68.h\
 ./emu68/ioplug68.h\
 ./emu68/macro68.h\
 ./emu68/mem68.h\
 ./emu68/srdef68.h\
 ./emu68/struct68.h\
 ./emu68/type68.h\
 ./file68/alloc68.h\
 ./file68/debugmsg68.h\
 ./file68/endian68.h\
 ./file68/error68.h\
 ./file68/file68.h\
 ./file68/gzip68.h\
 ./file68/ice68.h\
 ./file68/istream68.h\
 ./file68/istream68_def.h\
 ./file68/istream68_fd.h\
 ./file68/istream68_file.h\
 ./file68/istream68_mem.h\
 ./file68/rsc68.h\
 ./file68/string68.h\
 ./io68/io68.h\
 ./io68/mfp_io.h\
 ./io68/mfpemul.h\
 ./io68/mw_io.h\
 ./io68/mwemul.h\
 ./io68/paula_io.h\
 ./io68/paulaemul.h\
 ./io68/shifter_io.h\
 ./io68/ym_fixed_vol.h\
 ./io68/ym_io.h\
 ./io68/ymemul.h\
 ./io68/ymoutorg.h\
 ./unice68/unice68.h\
 ./config68.h\
 ./config_option68.h\
 ./config_package68.h\
 ./config_platform68.h\
 ./config_type68.h

nobase_pkginclude_HEADERS = $(my_install_headers)

PKGCONFIG_INSTALL_PATH=@PKGCONFIG_INSTALL_PATH@

install-pkg-config:
	$(mkinstalldirs) $(DESTDIR)$(PKGCONFIG_INSTALL_PATH)
	$(install_sh_DATA) sc68.pc $(DESTDIR)$(PKGCONFIG_INSTALL_PATH)

uninstall-pkg-config:
	-rm -f $(DESTDIR)$(PKGCONFIG_INSTALL_PATH)/sc68.pc

install-data-hook: install-pkg-config

uninstall-hook: uninstall-pkg-config
	test ! -d $(DESTDIR)$(pkgincludedir)\
	|| rm -rf  $(DESTDIR)$(pkgincludedir)
	test ! -d $(DESTDIR)$(pkgdatadir)\
	|| rm -rf  $(DESTDIR)$(pkgdatadir)
