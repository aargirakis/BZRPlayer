#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include "buttonoscilloscope.h"
#include "plugins.h"
#include "soundmanager.h"
#include "channels.h"

ButtonOscilloscope::ButtonOscilloscope(Channels *channelsProvided, const int i) {
    channelNumber = i;
    font.setFamily("Roboto");
    font.setPixelSize(14);
    checked = true;
    channels = channelsProvided;
}

bool ButtonOscilloscope::isChecked() const {
    return checked;
}

void ButtonOscilloscope::setEnabledColor(const QColor enabled) {
    enabledColor = enabled;
}

void ButtonOscilloscope::setDisabledColor(const QColor disabled) {
    disabledColor = disabled;
}

void ButtonOscilloscope::setBackgroundColor(const QColor background) {
    backgroundColor = background;
}

void ButtonOscilloscope::setTextColor(const QColor text) {
    textColor = text;
}

void ButtonOscilloscope::setChecked(const bool c) {
    checked = c;
    update();
}

void ButtonOscilloscope::paintEvent(QPaintEvent *event) {
    const auto &info = SoundManager::getInstance().info;

    if (info == nullptr) {
        return;
    }

    QPainter painter(this);
    painter.begin(this);

    if (info->plugin == PLUGIN_sndh_player) {
        painter.setBrush(QBrush(backgroundColor));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawRect(event->rect());

        constexpr int kLatencySampleCount = 256; // 44100 / 60;
        drawOscilloVoice(info->waveformDisplay, kLatencySampleCount, channelNumber);
        drawChannelNumber(QString::number(channelNumber));

        painter.end();
        update();
    } else {
        painter.setBrush(QBrush(checked ? enabledColor : disabledColor));
        painter.drawRect(event->rect());
        painter.end();
    }

    drawChannelNumber(QString::number(channelNumber));
}

void ButtonOscilloscope::drawChannelNumber(const QString &text) {
    QPainter painter(this);
    QPen pen;
    painter.setFont(font);
    pen.setColor(textColor);
    painter.setPen(pen);
    painter.drawText(5, 15, text);
}

void ButtonOscilloscope::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        checked = !checked;
        channels->muteChannels();
        update();
    }
}

void ButtonOscilloscope::drawOscilloVoice(const uint32_t *audio, const int count, const int index) {
    QPainter painter(this);

    constexpr float dpiScale = 1.0f;
    const int voiceShift = index * 8;

    int w = width();

    if (w > count)
        w = count;

    if (w <= 0)
        w = 1;

    int rPos = 0;
    const int rStep = (count << 16) / w;
    QPainterPath myPath;
    QPen pen;
    pen.setColor(enabledColor);
    pen.setWidth(2);
    painter.setPen(pen);

    for (int i = 0; i < w; i++) {
        float x = static_cast<float>(i) / static_cast<float>(w);
        float y = 0.f;

        if (audio) {
            const auto sv = static_cast<int8_t>(audio[rPos >> 16] >> voiceShift & 0xff);
            y = sv * (1.0f / 256.f);
            rPos += rStep;
        }

        //waveform[i] = ImLerp(inRect.Min, inRect.Max, ImVec2(x, 0.5f - y));
        //dl->AddPolyline(waveform, w, color, ImDrawFlags_None, dpiScale);

        //x=(x*width()*3.2)-8;

        x = x * width();
        y = -1 * (y * height() * 1.5) + height() / 2;

        if (i == 0) {
            myPath.moveTo(x, y);
        } else {
            myPath.lineTo(x, y);
        }
    }

    painter.drawPath(myPath);
}
