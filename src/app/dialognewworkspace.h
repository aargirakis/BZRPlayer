#ifndef DIALOGNEWWORKSPACE_H
#define DIALOGNEWWORKSPACE_H

#include <QDialog>

namespace Ui
{
    class DialogNewWorkspace;
}

class DialogNewWorkspace : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewWorkspace(QWidget* parent = nullptr);
    ~DialogNewWorkspace();

private slots:
    void on_buttonCancel_clicked();

    void on_buttonSave_clicked();

private:
    Ui::DialogNewWorkspace* ui;
};

#endif // DIALOGNEWWORKSPACE_H
