#include "dialognewworkspace.h"
#include "mainwindow.h"
#include "channels.h"
#include "ui_dialognewworkspace.h"

DialogNewWorkspace::DialogNewWorkspace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewWorkspace)
{
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);
    ui->lineEditNewWorkspace->setFocus();
}

DialogNewWorkspace::~DialogNewWorkspace()
{
    delete ui;
}

void DialogNewWorkspace::on_buttonCancel_clicked()
{
    close();
}


void DialogNewWorkspace::on_buttonSave_clicked()
{
    MainWindow* mw = static_cast<MainWindow*>(this->parent());
    if(!ui->lineEditNewWorkspace->text().isEmpty())
    {
        mw->CreateNewWorkspace(ui->lineEditNewWorkspace->text());
        close();
    }
}

