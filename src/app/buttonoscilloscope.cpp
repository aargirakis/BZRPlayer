#include "buttonoscilloscope.h"

#include <plugins.h>

#include "qevent.h"
#include "qpainter.h"
#include "qpainterpath.h"
#include "soundmanager.h"
#include "channels.h"
ButtonOscilloscope::ButtonOscilloscope(Channels* channels, int i)
{
    channelNumber=i;
    font.setFamily("Roboto");
    font.setPixelSize(14);
    checked=true;
    this->channels = channels;

}
bool ButtonOscilloscope::isChecked()
{
    return checked;
}
void ButtonOscilloscope::setEnabledColor(QColor enabled)
{
    enabledColor=enabled;
}
void ButtonOscilloscope::setDisabledColor(QColor disabled)
{
    disabledColor=disabled;
}
void ButtonOscilloscope::setBackgroundColor(QColor background)
{
    backgroundColor=background;
}
void ButtonOscilloscope::setTextColor(QColor text)
{
    textColor=text;
}
void ButtonOscilloscope::setChecked(bool c)
{
    checked=c;
    update();
}
void ButtonOscilloscope::paintEvent(QPaintEvent *event)
{
if(SoundManager::getInstance().m_Info1 == nullptr)
{
    return;
}
    //TODO refactor this shit
    if(checked || SoundManager::getInstance().m_Info1->plugin==PLUGIN_sndh_player)
    {
        if(SoundManager::getInstance().m_Info1->plugin==PLUGIN_sndh_player)
        {
            QPainter painter(this);
            painter.setBrush(QBrush(backgroundColor));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.begin(this);
            painter.drawRect(event->rect());

            QPainterPath myPath;

            int kLatencySampleCount = 256; //44100/60;
            drawOscilloVoice(SoundManager::getInstance().m_Info1->waveformDisplay,kLatencySampleCount,channelNumber);
            drawChannelNumber(QString::number(channelNumber));

            painter.end();
            update();
        }
        else
        {
            QPainter painter(this);
            painter.begin(this);
            painter.setBrush(QBrush(enabledColor));
            painter.drawRect(event->rect());
            painter.end();
        }
    }
    else
    {
        QPainter painter(this);
        painter.begin(this);
        painter.setBrush(QBrush(disabledColor));
        painter.drawRect(event->rect());
        painter.end();
    }
    drawChannelNumber(QString::number(channelNumber));


}
void ButtonOscilloscope::drawChannelNumber(QString text)
{
    QPainter painter(this);
    QPen pen;
    painter.setFont(font);
    pen.setColor(textColor);
    painter.setPen(pen);
    painter.drawText(5,15,text);
}
void ButtonOscilloscope::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        checked=!checked;
        channels->muteChannels();
        update();
    }
}
void ButtonOscilloscope::drawOscilloVoice(const uint32_t* audio, int count, int index)
{
    QPainter painter(this);


    const float dpiScale = 1.0f;
    const int voiceShift = index * 8;


        int w = width();
        if (w > count)
            w = count;
        if (w <= 0)
            w = 1;

        int rPos = 0;
        int rStep = (count << 16) / w;
        QPainterPath myPath;
        QPen pen;
        pen.setColor(enabledColor);
        pen.setWidth(2);
        painter.setPen(pen);

        //cout << "w: " << w  << "\n";
        //flush(cout);
        for (int i = 0; i < w; i++)
        {
            float x = (float)i / float(w);
            float y = 0.f;
            if (audio)
            {
                int8_t sv = int8_t((audio[rPos >> 16] >> voiceShift) & 0xff);
                y = float(sv * (1.0f / 256.f));
                rPos += rStep;
            }
            //waveform[i] = ImLerp(inRect.Min, inRect.Max, ImVec2(x, 0.5f - y));
            //dl->AddPolyline(waveform, w, color, ImDrawFlags_None, dpiScale);

            //x=(x*width()*3.2)-8;

            x=x*width();
            y=-1*(y*height()*1.5)+height()/2;

            if(i==0)
            {
                myPath.moveTo (x,y);
            }
            else
            {
                myPath.lineTo (x,y);
            }
            //cout << "Drawing " << x << "," << y << " DONE\n";
            //flush(cout);


        }
        //cout << "FOR LOOP END\n";
        //flush(cout);
        painter.drawPath(myPath);
        //cout << "drawPath\n";
        //flush(cout);



}
