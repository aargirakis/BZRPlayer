ifndef DO
$(error Use "Makefile" instead of "www.mk")
endif

WWW_TARGETS = www/index.html www/android.html www/windows.html www/macos.html www/linux.html \
	www/web.html www/news.html www/sap-format.html www/contact.html \
	www/apokeysnd.dll www/asap.js www/asapweb.js \
	www/favicon.ico www/PORTS.xml www/PORTS.xsl
CLEAN += $(WWW_TARGETS)

www: $(WWW_TARGETS)
.PHONY: www

www/%.html: $(call src,www/www.xsl www/%.xml)
	$(DO)xsltproc -o $@ --stringparam version $(VERSION) $^ && java -jar C:/bin/vnu.jar $@

www/apokeysnd.dll: win32/apokeysnd.dll
	$(COPY)

www/asap.js: javascript/asap.js
	$(COPY)

www/asapweb.js: $(srcdir)javascript/asapweb.js
	$(COPY)

www/favicon.ico: $(srcdir)win32/wasap/wasap.ico
	$(COPY)

www/PORTS.xml: $(srcdir)PORTS.xml
	$(COPY)

www/PORTS.xsl: $(srcdir)PORTS.xsl
	$(COPY)

sftp:
	sftp pfusik,asap@web.sourceforge.net:htdocs
.PHONY: sftp
