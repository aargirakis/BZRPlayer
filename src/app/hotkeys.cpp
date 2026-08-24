#include "hotkeys.h"
#include "ui_hotkeys.h"

hotkeys::hotkeys(QWidget *parent) : QDialog(parent),
                                ui(new Ui::hotkeys) {
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);
    mainWindow = static_cast<MainWindow *>(this->parent());
}

hotkeys::~hotkeys() {
    delete ui;
}

void hotkeys::on_buttonOk_clicked() {
    close();
}
