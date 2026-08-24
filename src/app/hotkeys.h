#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
    class MainWindow;
    class hotkeys;
}

class hotkeys : public QDialog {
    Q_OBJECT

public:
    explicit hotkeys(QWidget *parent = nullptr);

    ~hotkeys();

private slots:
    void on_buttonOk_clicked();

private:
    MainWindow *mainWindow;
    Ui::hotkeys *ui;
};

#endif // HOTKEYS_H
