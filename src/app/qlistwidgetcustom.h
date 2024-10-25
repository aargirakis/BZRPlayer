#ifndef QLISTWIDGETCUSTOM_H
#define QLISTWIDGETCUSTOM_H

#include <QListWidget>
#include <QWidget>

class QListWidgetCustom : public QListWidget
{
public:
    explicit QListWidgetCustom(QWidget* parent = 0);

    ~QListWidgetCustom()
    {
    }

protected:
    void dragMoveEvent(QDragMoveEvent* event) override;
};


#endif // QLISTWIDGETCUSTOM_H
