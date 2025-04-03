#include "mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
#ifndef WIN32
    // TODO temp workaround for qt-ads on wayland https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/714
    if (comparesEqual(qEnvironmentVariable("XDG_SESSION_TYPE"), "wayland")) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    QApplication a(argc, argv);
    MainWindow w(argc, argv);
    //w.show();

    return a.exec();
}
