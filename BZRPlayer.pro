
QT       += core gui svg xml opengl network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BZRPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        src/QTableWidgetCustom.cpp \
        src/about.cpp \
        src/album.cpp \
        src/albumgrid.cpp \
        src/buttonoscilloscope.cpp \
        src/channels.cpp \
        src/dialogdeleteworkspace.cpp \
        src/dialognewworkspace.cpp \
        src/filedownloader.cpp \
        src/fileinfoparser.cpp \
        src/main.cpp \
        src/mainwindow.cpp \
        src/myitemdelegate.cpp \
        src/playlistmodel.cpp \
        src/noPageSlider.cpp \
        src/patternview/AHXPatternView.cpp \
        src/patternview/AbstractPatternView.cpp \
        src/patternview/ChipTrackerPatternView.cpp \
        src/patternview/Composer669PatternView.cpp \
        src/patternview/DigiBooster17PatternView.cpp \
        src/patternview/DigiBoosterProPatternView.cpp \
        src/patternview/FastTracker1PatternView.cpp \
        src/patternview/FastTracker24ChanPatternView.cpp \
        src/patternview/FastTracker26ChanPatternView.cpp \
        src/patternview/FastTracker2PatternView.cpp \
        src/patternview/GameMusicCreatorPatternView.cpp \
        src/patternview/GenericPatternView.cpp \
        src/patternview/HivelyTrackerPatternView.cpp \
        src/patternview/IceTrackerPatternView.cpp \
        src/patternview/ImpulseTrackerPatternView.cpp \
        src/patternview/MEDPatternView.cpp \
        src/patternview/MultiTrackerPatternView.cpp \
        src/patternview/NoiseTrackerPatternView.cpp \
        src/patternview/OctaMED44ChanPatternView.cpp \
        src/patternview/OctaMED54ChanPatternView.cpp \
        src/patternview/OctaMED5ChanPatternView.cpp \
        src/patternview/OctaMEDPatternView.cpp \
        src/patternview/OctaMEDSoundstudioPatternView.cpp \
        src/patternview/OktalyzerPatternView.cpp \
        src/patternview/ProTracker1PatternView.cpp \
        src/patternview/ProTracker36PatternView.cpp \
        src/patternview/ScreamTracker2PatternView.cpp \
        src/patternview/ScreamTracker3PatternView.cpp \
        src/patternview/SoundFXPatternView.cpp \
        src/patternview/SoundTracker26PatternView.cpp \
        src/patternview/StarTrekker13PatternView.cpp \
        src/patternview/UltimateSoundTrackerPatternView.cpp \
        src/patternview/UltraTrackerPatternView.cpp \
        src/patternview/bitmapfont.cpp \
        src/playlist.cpp \
        src/playlistitem.cpp \
        src/qlistwidgetcustom.cpp \
        src/settingswindow.cpp \
        src/soundmanager.cpp \
        src/trackerview.cpp \
        src/various.cpp \
        src/visualizer.cpp \
        src/visualizers/effect.cpp \
        src/visualizers/parallax.cpp \
        src/visualizers/scroller.cpp \
        src/visualizers/tracker.cpp \
        src/visualizers/trackerfullscreen.cpp \
        src/visualizers/visualizerfullscreen.cpp

HEADERS += \
        src/QTableWidgetCustom.h \
        src/about.h \
        src/album.h \
        src/albumgrid.h \
        src/buttonoscilloscope.h \
        src/channels.h \
        src/dialogdeleteworkspace.h \
        src/dialognewworkspace.h \
        src/filedownloader.h \
        src/fileinfoparser.h \
        src/info.h \
        src/mainwindow.h \
        src/myitemdelegate.h \
        src/playlistmodel.h \
        src/noPageSlider.h \
        src/patternview/AHXPatternView.h \
        src/patternview/AbstractPatternView.h \
        src/patternview/ChipTrackerPatternView.h \
        src/patternview/Composer669PatternView.h \
        src/patternview/DigiBooster17PatternView.h \
        src/patternview/DigiBoosterProPatternView.h \
        src/patternview/FastTracker1PatternView.h \
        src/patternview/FastTracker24ChanPatternView.h \
        src/patternview/FastTracker26ChanPatternView.h \
        src/patternview/FastTracker2PatternView.h \
        src/patternview/GameMusicCreatorPatternView.h \
        src/patternview/GenericPatternView.h \
        src/patternview/HivelyTrackerPatternView.h \
        src/patternview/IceTrackerPatternView.h \
        src/patternview/ImpulseTrackerPatternView.h \
        src/patternview/MEDPatternView.h \
        src/patternview/MultiTrackerPatternView.h \
        src/patternview/NoiseTrackerPatternView.h \
        src/patternview/OctaMED44ChanPatternView.h \
        src/patternview/OctaMED54ChanPatternView.h \
        src/patternview/OctaMED5ChanPatternView.h \
        src/patternview/OctaMEDPatternView.h \
        src/patternview/OctaMEDSoundstudioPatternView.h \
        src/patternview/OktalyzerPatternView.h \
        src/patternview/PolyTrackerPatternView.h \
        src/patternview/ProTracker1PatternView.h \
        src/patternview/ProTracker36PatternView.h \
        src/patternview/ScreamTracker2PatternView.h \
        src/patternview/ScreamTracker3PatternView.h \
        src/patternview/SoundFXPatternView.h \
        src/patternview/SoundTracker26PatternView.h \
        src/patternview/StarTrekker13PatternView.h \
        src/patternview/UltimateSoundTrackerPatternView.h \
        src/patternview/UltraTrackerPatternView.h \
        src/patternview/bitmapfont.h \
        src/playlist.h \
        src/playlistitem.h \
        src/qlistwidgetcustom.h \
        src/settingswindow.h \
        src/soundmanager.h \
        src/trackerview.h \
        src/various.h \
        src/visualizer.h \
        src/visualizers/effect.h \
        src/visualizers/parallax.h \
        src/visualizers/scroller.h \
        src/visualizers/tracker.h \
        src/visualizers/trackerfullscreen.h \
        src/visualizers/visualizerfullscreen.h

FORMS += \
        src/about.ui \
        src/dialogdeleteworkspace.ui \
        src/dialognewworkspace.ui \
        src/mainwindow.ui \
        src/settingswindow.ui

win32: LIBS += -L$$PWD/external/FMOD/api/core/lib/x86/ -lfmod
LIBS += -L$$PWD/external/Qt-Advanced-Docking-System-4.0.0/bin -llibqtadvanceddocking
INCLUDEPATH += external/FMOD/api/core/inc \
patternview/ \
external/Qt-Advanced-Docking-System-4.0.0/src

DEPENDPATH += external/FMOD/api/core/inc \
$$PWD/external/Qt-Advanced-Docking-System-4.0.0/src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src/resource.qrc

CONFIG -= embed_manifest_exe

RC_FILE = src/BZRPlayer.rc
