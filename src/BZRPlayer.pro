
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
        QTableWidgetCustom.cpp \
        about.cpp \
        album.cpp \
        albumgrid.cpp \
        buttonoscilloscope.cpp \
        channels.cpp \
        dialogdeleteworkspace.cpp \
        dialognewworkspace.cpp \
        filedownloader.cpp \
        fileinfoparser.cpp \
        main.cpp \
        mainwindow.cpp \
        myitemdelegate.cpp \
        playlistmodel.cpp \
        noPageSlider.cpp \
        patternview/AHXPatternView.cpp \
        patternview/AbstractPatternView.cpp \
        patternview/ChipTrackerPatternView.cpp \
        patternview/Composer669PatternView.cpp \
        patternview/DigiBooster17PatternView.cpp \
        patternview/DigiBoosterProPatternView.cpp \
        patternview/FastTracker1PatternView.cpp \
        patternview/FastTracker24ChanPatternView.cpp \
        patternview/FastTracker26ChanPatternView.cpp \
        patternview/FastTracker2PatternView.cpp \
        patternview/GameMusicCreatorPatternView.cpp \
        patternview/GenericPatternView.cpp \
        patternview/HivelyTrackerPatternView.cpp \
        patternview/IceTrackerPatternView.cpp \
        patternview/ImpulseTrackerPatternView.cpp \
        patternview/MEDPatternView.cpp \
        patternview/MultiTrackerPatternView.cpp \
        patternview/NoiseTrackerPatternView.cpp \
        patternview/OctaMED44ChanPatternView.cpp \
        patternview/OctaMED54ChanPatternView.cpp \
        patternview/OctaMED5ChanPatternView.cpp \
        patternview/OctaMEDPatternView.cpp \
        patternview/OctaMEDSoundstudioPatternView.cpp \
        patternview/OktalyzerPatternView.cpp \
        patternview/ProTracker1PatternView.cpp \
        patternview/ProTracker36PatternView.cpp \
        patternview/ScreamTracker2PatternView.cpp \
        patternview/ScreamTracker3PatternView.cpp \
        patternview/SoundFXPatternView.cpp \
        patternview/SoundTracker26PatternView.cpp \
        patternview/StarTrekker13PatternView.cpp \
        patternview/UltimateSoundTrackerPatternView.cpp \
        patternview/UltraTrackerPatternView.cpp \
        patternview/bitmapfont.cpp \
        playlist.cpp \
        playlistitem.cpp \
        qlistwidgetcustom.cpp \
        settingswindow.cpp \
        soundmanager.cpp \
        trackerview.cpp \
        various.cpp \
        visualizer.cpp \
        visualizers/effect.cpp \
        visualizers/parallax.cpp \
        visualizers/scroller.cpp \
        visualizers/tracker.cpp \
        visualizers/trackerfullscreen.cpp \
        visualizers/visualizerfullscreen.cpp

HEADERS += \
        QTableWidgetCustom.h \
        about.h \
        album.h \
        albumgrid.h \
        buttonoscilloscope.h \
        channels.h \
        dialogdeleteworkspace.h \
        dialognewworkspace.h \
        filedownloader.h \
        fileinfoparser.h \
        info.h \
        mainwindow.h \
        myitemdelegate.h \
        playlistmodel.h \
        noPageSlider.h \
        patternview/AHXPatternView.h \
        patternview/AbstractPatternView.h \
        patternview/ChipTrackerPatternView.h \
        patternview/Composer669PatternView.h \
        patternview/DigiBooster17PatternView.h \
        patternview/DigiBoosterProPatternView.h \
        patternview/FastTracker1PatternView.h \
        patternview/FastTracker24ChanPatternView.h \
        patternview/FastTracker26ChanPatternView.h \
        patternview/FastTracker2PatternView.h \
        patternview/GameMusicCreatorPatternView.h \
        patternview/GenericPatternView.h \
        patternview/HivelyTrackerPatternView.h \
        patternview/IceTrackerPatternView.h \
        patternview/ImpulseTrackerPatternView.h \
        patternview/MEDPatternView.h \
        patternview/MultiTrackerPatternView.h \
        patternview/NoiseTrackerPatternView.h \
        patternview/OctaMED44ChanPatternView.h \
        patternview/OctaMED54ChanPatternView.h \
        patternview/OctaMED5ChanPatternView.h \
        patternview/OctaMEDPatternView.h \
        patternview/OctaMEDSoundstudioPatternView.h \
        patternview/OktalyzerPatternView.h \
        patternview/PolyTrackerPatternView.h \
        patternview/ProTracker1PatternView.h \
        patternview/ProTracker36PatternView.h \
        patternview/ScreamTracker2PatternView.h \
        patternview/ScreamTracker3PatternView.h \
        patternview/SoundFXPatternView.h \
        patternview/SoundTracker26PatternView.h \
        patternview/StarTrekker13PatternView.h \
        patternview/UltimateSoundTrackerPatternView.h \
        patternview/UltraTrackerPatternView.h \
        patternview/bitmapfont.h \
        playlist.h \
        playlistitem.h \
        qlistwidgetcustom.h \
        settingswindow.h \
        soundmanager.h \
        trackerview.h \
        various.h \
        visualizer.h \
        visualizers/effect.h \
        visualizers/parallax.h \
        visualizers/scroller.h \
        visualizers/tracker.h \
        visualizers/trackerfullscreen.h \
        visualizers/visualizerfullscreen.h

FORMS += \
        about.ui \
        dialogdeleteworkspace.ui \
        dialognewworkspace.ui \
        mainwindow.ui \
        settingswindow.ui

win32: LIBS += -L$$PWD/../../../../FMOD/api/core/lib/x86/ -lfmod
LIBS += -L$$PWD/external/Qt-Advanced-Docking-System-4.0.0/bin -llibqtadvanceddocking
INCLUDEPATH += $$PWD/../../../../FMOD/api/core/inc \
../../../../libmodplug/ \
../../../../includes/ \
patternview/ \
external/Qt-Advanced-Docking-System-4.0.0/src

DEPENDPATH += $$PWD/../../../../FMOD/api/core/inc \
$$PWD/external/Qt-Advanced-Docking-System-4.0.0/src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

RC_ICONS = resources/icon.ico
