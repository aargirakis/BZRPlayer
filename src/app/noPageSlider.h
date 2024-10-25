#ifndef NOPAGESLIDER_H
#define NOPAGESLIDER_H

#include <QSlider>

class NoPageSlider : public QSlider
{
    Q_OBJECT

public:
    NoPageSlider(QWidget* parent = 0);


    void setDefaultValue(int defaultValue) { this->defValue = defaultValue; }
    int defaultValue() { return defValue; }
    bool snapToDefault() { return snapDefault; }
    void setSnapToDefault(bool snap) { this->snapDefault = snap; }
    void setSnapDistance(int dist) { this->snapDistance = dist; }

protected:
    //QValidator::State validate(QString &text, int &pos) const;
    //int valueFromText(const QString &text) const;
    //QString textFromValue(int value) const;
    void mousePressEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);

private:
    //QRegExpValidator *validator;
    int valueFromPoint(const QPoint& p);
    QStyleOptionSlider getStyleOption() const;
    int defValue;
    bool snapDefault;
    int snapDistance;
};

#endif
