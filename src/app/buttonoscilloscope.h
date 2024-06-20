#ifndef BUTTONOSCILLOSCOPE_H
#define BUTTONOSCILLOSCOPE_H



#include <QOpenGLWidget>
#include <QObject>
#include <QWidget>
class Channels;
class ButtonOscilloscope : public QOpenGLWidget
{
    Q_OBJECT
public:
    ButtonOscilloscope(Channels*, int);
    bool isChecked();
    void setChecked(bool);
    void drawOscilloVoice(const uint32_t* audio, int count, int indexQ);
    void drawChannelNumber(QString text);
    void setEnabledColor(QColor);
    void setDisabledColor(QColor);
    void setTextColor(QColor);
    void setBackgroundColor(QColor);


private:
    bool checked;
    int channelNumber;
    QFont font;
    Channels* channels;
    QColor enabledColor;
    QColor disabledColor;
    QColor textColor;
    QColor backgroundColor;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent * e);


};

#endif // BUTTONOSCILLOSCOPE_H
