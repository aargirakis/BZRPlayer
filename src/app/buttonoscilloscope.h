#ifndef BUTTONOSCILLOSCOPE_H
#define BUTTONOSCILLOSCOPE_H

#include <QOpenGLWidget>

class Channels;

class ButtonOscilloscope : public QOpenGLWidget {
    Q_OBJECT

public:
    ButtonOscilloscope(Channels *, int);

    bool isChecked() const;

    void setChecked(bool);

    void drawOscilloVoice(const uint32_t *audio, int count, int index);

    void drawChannelNumber(const QString &text);

    void setEnabledColor(QColor);

    void setDisabledColor(QColor);

    void setTextColor(QColor);

    void setBackgroundColor(QColor);

private:
    bool checked;
    int channelNumber;
    QFont font;
    Channels *channels;
    QColor enabledColor;
    QColor disabledColor;
    QColor textColor;
    QColor backgroundColor;

protected:
    void paintEvent(QPaintEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // BUTTONOSCILLOSCOPE_H
