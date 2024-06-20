GIT = git
TAR = /usr/bin/tar
UNIX2DOS = $(DO)unix2dos <$< >$@
GREP = @grep -H

# no user-configurable paths below this line

VERSION = 5.2.0

ifndef DO
$(error Use "Makefile" instead of "release.mk")
endif

dist: \
	release/asap-$(VERSION)-android.apk \
	release/asap-$(VERSION)-web.zip \
	release/asap-$(VERSION)-win32.zip \
	release/signed-msi \
	release/foo_asap-$(VERSION).fb2k-component \
	srcdist
.PHONY: dist

srcdist: $(srcdir)MANIFEST $(srcdir)asap.c $(srcdir)asap.h $(ASM6502_OBX)
	$(RM) release/asap-$(VERSION).tar.gz && $(TAR) -c --numeric-owner --owner=0 --group=0 --mode=644 -T MANIFEST --transform=s,,asap-$(VERSION)/, | $(SEVENZIP) -tgzip -si release/asap-$(VERSION).tar.gz
.PHONY: srcdist

$(srcdir)MANIFEST:
	$(DO)if test -e $(srcdir).git; then \
		($(GIT) ls-files | grep -vF .gitignore \
			&& echo MANIFEST && echo asap.c && echo asap.h \
			&& for obx in $(ASM6502_OBX); do echo $$obx; done) | /usr/bin/sort -u >$@; \
	fi
.PHONY: $(srcdir)MANIFEST

release/asap-$(VERSION)-web.zip: release/COPYING.txt \
	javascript/asap.js $(srcdir)javascript/asapweb.js
	$(MAKEZIP)

release/asap-$(VERSION)-win32.zip: release/COPYING.txt \
	$(addprefix win32/,asapconv.exe asapscan.exe wasap.exe in_asap.dll foo_asap.dll apokeysnd.dll xmp-asap.dll bass_asap.dll ASAPShellEx.dll libasap_plugin.dll signed)
	$(MAKEZIP)

release/foo_asap-$(VERSION).fb2k-component: win32/foo_asap.dll win32/signed
	$(MAKEZIP)

release/asap-$(VERSION)-macos.dmg: release/osx/libasap_plugin.dylib release/osx/plugins release/osx/asapconv release/osx/bin
ifdef PORK_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(PORK_CODESIGNING_IDENTITY) release/osx/libasap_plugin.dylib
	codesign --options runtime -f -s $(PORK_CODESIGNING_IDENTITY) release/osx/asapconv
endif
	$(DO)hdiutil create -volname asap-$(VERSION)-macos -srcfolder release/osx -format UDBZ -fs HFS+ -imagekey bzip2-level=3 -ov $@
ifdef PORK_NOTARIZING_CREDENTIALS
	xcrun altool --notarize-app --primary-bundle-id net.sf.asap $(PORK_NOTARIZING_CREDENTIALS) --file $@ \
		| perl -pe 's/^RequestUUID =/xcrun altool $$ENV{PORK_NOTARIZING_CREDENTIALS} --notarization-info/ or next; $$c = $$_; until (/Status: success/) { sleep 20; $$_ = `$$c`; print; } last;'
endif

release/osx/libasap_plugin.dylib: libasap_plugin.dylib
	$(DO)strip -o $@ -x $< && chmod 644 $@
CLEANDIR += release/osx

release/osx/plugins:
	$(DO)ln -s /Applications/VLC.app/Contents/MacOS/plugins $@

release/osx/asapconv: $(call src,asapconv.c asap.[ch])
	$(OSX_CC)

release/osx/bin:
	$(DO)ln -s /usr/local/bin $@

deb:
	debuild -b -us -uc
.PHONY: deb

deb64:
	scp release/asap-$(VERSION).tar.gz vm:.
	ssh vm 'rm -rf asap-$(VERSION) && tar xf asap-$(VERSION).tar.gz && make -C asap-$(VERSION) deb'
	scp vm:asap_$(VERSION)-1_amd64.deb release/
	scp vm:asap-dev_$(VERSION)-1_amd64.deb release/
	scp vm:asap-vlc_$(VERSION)-1_amd64.deb release/
	scp vm:asap-xmms2_$(VERSION)-1_amd64.deb release/
.PHONY: deb64

rpm64:
	scp release/asap-$(VERSION).tar.gz vm:.
	ssh vm 'rpmbuild -tb asap-$(VERSION).tar.gz'
	scp vm:rpmbuild/RPMS/x86_64/asap-$(VERSION)-1.x86_64.rpm release/
	scp vm:rpmbuild/RPMS/x86_64/asap-devel-$(VERSION)-1.x86_64.rpm release/
	scp vm:rpmbuild/RPMS/x86_64/asap-vlc-$(VERSION)-1.x86_64.rpm release/
	scp vm:rpmbuild/RPMS/x86_64/asap-xmms2-$(VERSION)-1.x86_64.rpm release/
.PHONY: rpm64

mac:
	scp release/asap-$(VERSION).tar.gz mac:.
	ssh mac 'security unlock-keychain ~/Library/Keychains/login.keychain && rm -rf asap-$(VERSION) && tar xf asap-$(VERSION).tar.gz && make -C asap-$(VERSION) release/asap-$(VERSION)-macos.dmg'
	scp mac:asap-$(VERSION)/release/asap-$(VERSION)-macos.dmg release/
.PHONY: mac

release/COPYING.txt: $(srcdir)COPYING
	$(UNIX2DOS)
CLEAN += release/COPYING.txt

version:
	@echo ./release/release.mk: VERSION=$(VERSION)
	$(GREP) -m 1 version= $(srcdir)www/news.xml
	$(GREP) "<since>" $(srcdir)PORTS.xml | /usr/bin/sort -ru | head -1
	$(GREP) Version: $(srcdir)asap.spec
	$(GREP) -m 1 ^asap $(srcdir)debian/changelog
	$(GREP) "int Version" $(srcdir)asapinfo.ci
	$(GREP) "VERSION =" $(srcdir)chksap.pl
	$(GREP) android:versionName $(srcdir)java/android/AndroidManifest.xml
	$(GREP) ", v" $(srcdir)win32/rmt/apokeysnd_dll.c
.PHONY: version
