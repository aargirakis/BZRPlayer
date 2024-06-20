#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QString>
#include "mainwindow.h"
namespace Ui {
class MainWindow;
class about;

}

class about : public QDialog
{
    Q_OBJECT

public:
    explicit about(QWidget *parent = nullptr);
    ~about();
    void setVersion(QString);

private:


private slots:


    void on_pushButton_clicked();

private:
    MainWindow *mainWindow;
    Ui::about *ui;

};

#endif // ABOUT_H
