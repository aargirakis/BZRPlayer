#include <QDir>
#include <QMessageBox>
#include "dialogdeleteworkspace.h"
#include "mainwindow.h"
#include "ui_dialogdeleteworkspace.h"

DialogDeleteWorkspace::DialogDeleteWorkspace(QWidget *parent) : QDialog(parent),
                                                                ui(new Ui::DialogDeleteWorkspace) {
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);

    const QDir directory(userPath + LAYOUTS_DIR);
    QStringList workspaces = directory.entryList(QStringList() << "*.ini", QDir::Files);

    foreach(QString filename, workspaces) {
        QFileInfo fileInfo(filename);
        QString basename = fileInfo.baseName();
        ui->comboBoxWorkspace->addItem(basename);
    }

    ui->comboBoxWorkspace->setFocus();
}

DialogDeleteWorkspace::~DialogDeleteWorkspace() {
    delete ui;
}

void DialogDeleteWorkspace::on_buttonCancel_clicked() {
    close();
}

void DialogDeleteWorkspace::on_buttonDelete_clicked() {
    if (const QString filename = ui->comboBoxWorkspace->currentText() + ".ini";
        QFile::remove(userPath + LAYOUTS_DIR + "/" + filename)) {
        const auto mw = static_cast<MainWindow *>(this->parent());
        mw->deleteWorkspace(ui->comboBoxWorkspace->currentText());
        ui->comboBoxWorkspace->removeItem(ui->comboBoxWorkspace->currentIndex());
        close();
    } else {
        QMessageBox::critical(this, "Error", "Couldn't delete layout");
    }
}
