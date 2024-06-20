XMMS2_CFLAGS = `pkg-config --cflags xmms2-plugin glib-2.0`
XMMS2_PLUGIN_DIR = `pkg-config --variable=libdir xmms2-plugin`/xmms2

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "xmms2.mk")
endif

asap-xmms2: libxmms_asap.so
.PHONY: asap-xmms2

libxmms_asap.so: $(call src,xmms2/libxmms_asap.c xmms2/xmms_configuration.h asap.[ch])
	$(DO_CC) $(XMMS2_CFLAGS)
CLEAN += libxmms_asap.so

install-xmms2: libxmms_asap.so
	$(call INSTALL_PROGRAM,libxmms_asap.so,$(XMMS2_PLUGIN_DIR))
.PHONY: install-xmms2

uninstall-xmms2:
	$(RM) $(DESTDIR)$(XMMS2_PLUGIN_DIR)/libxmms_asap.so
.PHONY: uninstall-xmms2
