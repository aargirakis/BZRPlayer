#include "dialogdeleteworkspace.h"
#include "qdir.h"
#include "mainwindow.h"
#include "ui_dialogdeleteworkspace.h"
#include <QMessageBox>

DialogDeleteWorkspace::DialogDeleteWorkspace(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DialogDeleteWorkspace)
{
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);

    QDir directory(QApplication::applicationDirPath() + "/user/layouts");
    QStringList workspaces = directory.entryList(QStringList() << "*.ini", QDir::Files);
    foreach(QString filename, workspaces)
    {
        QFileInfo fileInfo(filename);
        QString basename = fileInfo.baseName();
        ui->comboBoxWorkspace->addItem(basename);
    }
    ui->comboBoxWorkspace->setFocus();
}

DialogDeleteWorkspace::~DialogDeleteWorkspace()
{
    delete ui;
}

void DialogDeleteWorkspace::on_buttonCancel_clicked()
{
    close();
}


void DialogDeleteWorkspace::on_buttonDelete_clicked()
{
    QString fileName = ui->comboBoxWorkspace->currentText() + ".ini";
    bool removed = QFile::remove(QApplication::applicationDirPath() + "/user/layouts/" + fileName);
    if (removed)
    {
        MainWindow* mw = static_cast<MainWindow*>(this->parent());
        mw->DeleteWorkspace(ui->comboBoxWorkspace->currentText());
        ui->comboBoxWorkspace->removeItem(ui->comboBoxWorkspace->currentIndex());
        close();
    }
    else
    {
        QMessageBox::critical(this, "Error", "Couldn't delete layout.");
    }
}
