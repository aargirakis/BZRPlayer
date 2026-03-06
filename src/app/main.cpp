#include "mainwindow.h"
#include <QApplication>
#include <QProxyStyle>

class NoMenuBarAltKeyNavigationStyle : public QProxyStyle {
public:
    int styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget,
                  QStyleHintReturn *returnData) const override {
        if (stylehint == SH_MenuBar_AltKeyNavigation) {
            return 0;
        }

        return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
    }
};

int main(int argc, char* argv[])
{
#ifndef WIN32
    // TODO temp workaround for qt-ads on wayland https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/714
    if (QString::compare(qEnvironmentVariable("XDG_SESSION_TYPE"), "wayland", Qt::CaseSensitive) == 0) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    // see https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/732
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

#ifdef WIN32
    QApplication::setStyle(new NoMenuBarAltKeyNavigationStyle());
#endif

    QApplication a(argc, argv);
    MainWindow w(argc, argv);
    //w.show();

    return a.exec();
}
