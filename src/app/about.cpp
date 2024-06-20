#include "about.h"
#include "playlistmodel.h"
#include "ui_about.h"

about::about(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::about)
{
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);
    mainWindow = static_cast<MainWindow*>(this->parent());
    setVersion(mainWindow->getVersion());

}

about::~about()
{
    delete ui;
}


void about::setVersion(QString version)
{
    ui->labelVersion->setText(version);
}

void about::on_pushButton_clicked()
{
    close();
}

