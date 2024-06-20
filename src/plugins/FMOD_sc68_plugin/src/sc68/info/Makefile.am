info_TEXINFOS = sc68.texi
sc68_TEXINFOS = \
 sc68_as68.texi\
 sc68_configfile.texi\
 sc68_debug68.texi\
 sc68_info68.texi\
 sc68_install.texi\
 sc68_overview.texi\
 sc68_player.texi\
 sc68_sourcer68.texi\
 sc68_status.texi\
 sc68_unice68.texi

sc68_status.texi: Makefile
	( $(ECHO) "@set DATADIR $(pkgdatadir)";\
	$(ECHO) "@set BINDIR $(bindir)";\
	$(ECHO) "@set SC68URL $(SC68URL)";\
	) > $@

clean-generic:
	@rm -f sc68_status.texi
