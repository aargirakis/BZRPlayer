install_flac:
	$(call copyfile_cmd,$(dirs.root)/3rdparty/FLAC/$(arch)/FLAC.dll,$(DESTDIR))
