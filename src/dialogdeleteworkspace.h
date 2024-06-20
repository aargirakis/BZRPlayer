#ifndef DIALOGDELETEWORKSPACE_H
#define DIALOGDELETEWORKSPACE_H

#include <QDialog>

namespace Ui {
class DialogDeleteWorkspace;
}

class DialogDeleteWorkspace : public QDialog
{
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
