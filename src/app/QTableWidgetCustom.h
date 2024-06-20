#ifndef QTABLEWIDGETCUSTOM_H
#define QTABLEWIDGETCUSTOM_H

#include <QKeyEvent>
#include <QTableWidget>

class QTableWidgetCustom : public QTableWidget
{
    Q_OBJECT
public:
    explicit QTableWidgetCustom(QWidget *parent = 0);
    ~QTableWidgetCustom(){}

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // QTABLEWIDGETCUSTOM_H
