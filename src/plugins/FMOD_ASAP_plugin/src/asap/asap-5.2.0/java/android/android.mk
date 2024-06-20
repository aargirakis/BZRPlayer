ASMA_DIR = ../aasma/asma
ANDROID_SDK = C:/Users/fox/AppData/Local/Android/Sdk
ANDROID_JAR = $(ANDROID_SDK)/platforms/android-31/android.jar
ANDROID_BUILD_TOOLS = $(ANDROID_SDK)/build-tools/30.0.2

JAVA = $(DO)java
AAPT = $(ANDROID_BUILD_TOOLS)/aapt
D8 = $(DO)java -cp "$(ANDROID_BUILD_TOOLS)/lib/d8.jar" com.android.tools.r8.D8
JARSIGNER = $(DO)$(JAVA_SDK)/bin/jarsigner -sigalg SHA1withRSA -digestalg SHA1
ZIPALIGN = $(DO)$(ANDROID_BUILD_TOOLS)/zipalign
ADB = $(ANDROID_SDK)/platform-tools/adb
ANDROID = $(ANDROID_SDK)/tools/android.bat
EMULATOR = $(ANDROID_SDK)/tools/emulator

# NDK is only needed for the command-line asapconv
# It ended up in this Makefile even though it's not Java
ANDROID_NDK = C:/bin/android-ndk-r8c
ANDROID_NDK_PLATFORM = $(ANDROID_NDK)/platforms/android-3/arch-arm
ANDROID_CC = $(DO)$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-gcc --sysroot=$(ANDROID_NDK_PLATFORM) -s -O2 -Wall -o $@ $(INCLUDEOPTS) $(filter %.c,$^)

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "android.mk")
endif

ANDROID_RELEASE = release/asap-$(VERSION)-android.apk
ANDROID_JAVA_SRC = $(addprefix $(srcdir)java/android/, AATRFileInputStream.java ArchiveSelector.java ArchiveSuggestionsProvider.java BaseSelector.java \
	FileContainer.java FileInfo.java FileSelector.java JavaAATR.java Player.java PlayerService.java Util.java ZipInputStream.java)

android-release: $(ANDROID_RELEASE)
.PHONY: android-release

android-install-emu: $(ANDROID_RELEASE)
	$(ADB) -e install -r $<
.PHONY: android-install-emu

android-install-dev: $(ANDROID_RELEASE)
	$(ADB) -d install -r $<
.PHONY: android-install-dev

android-log-emu:
	$(ADB) -e logcat -d
.PHONY: android-log-emu

android-log-dev:
	$(ADB) -d logcat
.PHONY: android-log-dev

android-shell-emu:
	$(ADB) -e shell
.PHONY: android-shell-emu

android-shell-dev:
	$(ADB) -d shell
.PHONY: android-shell-dev

android-emu:
	$(EMULATOR) -avd kit &
.PHONY: android-emu

android-push-release: $(ANDROID_RELEASE)
	$(ADB) -d push $(ANDROID_RELEASE) /sdcard/sap/
.PHONY: android-push-release

android-push-sap:
	$(ADB) -e push ../Drunk_Chessboard.sap /sdcard/
.PHONY: android-push-sap

android-create-avd:
	$(ANDROID) create avd -n kit -t android-19 -c 16M
.PHONY: android-create-avd

$(ANDROID_RELEASE): java/android/AndroidASAP-unaligned.apk
	$(ZIPALIGN) -f 4 $< $@

java/android/AndroidASAP-unaligned.apk: java/android/AndroidASAP-unsigned.apk
	$(JARSIGNER) -storepass walsie -signedjar $@ $< pfusik
CLEAN += java/android/AndroidASAP-unaligned.apk

java/android/AndroidASAP-unsigned.apk: java/android/AndroidASAP-resources.apk java/android/classes.dex
	$(DO)cp $< $@ && $(SEVENZIP) -tzip $@ ./java/android/classes.dex
CLEAN += java/android/AndroidASAP-unsigned.apk

java/android/classes.dex: java/android/classes/net/sf/asap/Player.class
	$(D8) --release --output $(@D) --lib $(ANDROID_JAR) `ls java/android/classes/net/sf/asap/*.class`
CLEAN += java/android/classes.dex

java/android/classes/net/sf/asap/Player.class: $(ANDROID_JAVA_SRC) java/android/AndroidASAP-resources.apk java/src/net/sf/asap/ASAP.java
	$(JAVAC) -d java/android/classes --release 11 -cp $(ANDROID_JAR) -Xlint:deprecation $(ANDROID_JAVA_SRC) java/android/gen/net/sf/asap/R.java java/src/net/sf/asap/*.java
CLEANDIR += java/android/classes

# Also generates java/android/gen/net/sf/asap/R.java
java/android/AndroidASAP-resources.apk: $(addprefix $(srcdir)java/android/,AndroidManifest.xml res/drawable/ic_menu_browse.png res/drawable/icon.png res/layout/fileinfo_list_item.xml res/layout/filename_list_item.xml res/layout/playing.xml res/menu/archive_selector.xml res/menu/file_selector.xml res/values/strings.xml res/values/themes.xml res/xml/searchable.xml) $(ASMA_DIR)/index.txt $(JAVA_OBX)
	$(DO)mkdir -p java/android/gen && $(AAPT) p -f -m -M $< -I $(ANDROID_JAR) -S $(srcdir)java/android/res -A $(ASMA_DIR) --ignore-assets Docs:new.m3u -F $@ -J java/android/gen java/obx
CLEAN += java/android/AndroidASAP-resources.apk java/android/gen/net/sf/asap/R.java

$(ASMA_DIR)/index.txt: java/android/Indexer.class
	$(JAVA) -classpath "java/android;java/classes" Indexer $(ASMA_DIR) | dos2unix >$@
CLEAN += $(ASMA_DIR)/index.txt

java/android/Indexer.class: $(srcdir)java/android/Indexer.java java/classes/net/sf/asap/ASAP.class
	$(JAVAC) -d $(@D) -classpath java/classes $<
CLEAN += java/android/Indexer.class

android-push-asapconv: java/android/asapconv
	$(ADB) -d push java/android/asapconv /data/local/tmp/
.PHONY: android-push-asapconv

java/android/asapconv: $(call src,asapconv.c asap.[ch])
	$(ANDROID_CC)
CLEAN += java/android/asapconv
