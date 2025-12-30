#ifndef DIALOGDELETEWORKSPACE_H
#define DIALOGDELETEWORKSPACE_H

#include <QDialog>
#include "various.h"

namespace Ui {
    class DialogDeleteWorkspace;
}

class DialogDeleteWorkspace : public QDialog {
    PROVIDE_CLASS_NAME()

    Q_OBJECT

public:
    explicit DialogDeleteWorkspace(QWidget *parent = nullptr);

    ~DialogDeleteWorkspace();

private slots:
    void on_buttonCancel_clicked();

    void on_buttonDelete_clicked();

private:
    Ui::DialogDeleteWorkspace *ui;
};

#endif // DIALOGDELETEWORKSPACE_H
