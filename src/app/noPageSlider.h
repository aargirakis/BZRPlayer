#ifndef NOPAGESLIDER_H
#define NOPAGESLIDER_H

#include <QSlider>

class NoPageSlider : public QSlider
{
    Q_OBJECT

public:
    NoPageSlider(QWidget* parent = nullptr);

    void setDefaultValue(const int value) { this->defaultValue = value; }
    int getDefaultValue() const { return defaultValue; }
    bool snapToDefault() const { return snapDefault; }
    void setSnapToDefault(const bool snap) { this->snapDefault = snap; }

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
    int valueFromPoint(const QPoint& p) const;
    QStyleOptionSlider getStyleOption() const;
    int defaultValue;
    bool snapDefault;
    int snapDistance;
};

#endif
