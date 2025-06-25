#include "NoiseTrackerPatternView.h"

NoiseTrackerPatternView::NoiseTrackerPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font2 = QFont("Noisetracker 1.1 Double Height");
    m_font2.setPixelSize(14);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(0, 69, 222);
    m_font = QFont("Noisetracker 1.1");
    m_font.setPixelSize(7);
    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Noisetracker 1.1");
    m_fontWidth = 8;
    m_fontHeight = 7;
    m_bitmapFont2 = BitmapFont("Protracker 1.0 Double Height");

    m_renderTop = true;
    m_renderVUMeter = true;

    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 8;

    m_xChannelStart = 29;
    m_channelWidth = 70;
    m_channelLastWidth = 72;
    m_channelxSpace = 2;

    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 33;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 33;


    setupVUMeters();


    //main color
    m_linearGrad.setColorAt(0, QColor(189, 16, 0).rgb()); //red
    m_linearGrad.setColorAt(0.11, QColor(189, 50, 0).rgb()); //red
    m_linearGrad.setColorAt(0.36, QColor(189, 255, 0).rgb()); //yellow
    m_linearGrad.setColorAt(0.81, QColor(0, 255, 0).rgb()); // green
    m_linearGrad.setColorAt(0.87, QColor(0, 227, 0).rgb()); // green
    m_linearGrad.setColorAt(0.95, QColor(0, 169, 0).rgb()); // green


    //hilight color (left)
    m_linearGradHiLite.setColorAt(0, QColor(255, 16, 0).rgb()); //red
    m_linearGradHiLite.setColorAt(0.11, QColor(255, 50, 0).rgb()); //red
    m_linearGradHiLite.setColorAt(0.36, QColor(255, 255, 0).rgb()); //yellow
    m_linearGradHiLite.setColorAt(0.81, QColor(66, 255, 0).rgb()); // green
    m_linearGradHiLite.setColorAt(1, QColor(0, 239, 0).rgb()); // dark green

    //dark color (right)
    m_linearGradDark.setColorAt(0, QColor(115, 16, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.11, QColor(111, 50, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.36, QColor(115, 255, 0).rgb()); //yellow
    m_linearGradDark.setColorAt(0.74, QColor(0, 218, 0).rgb()); // green
    m_linearGradDark.setColorAt(0.85, QColor(0, 166, 0).rgb()); // green
    m_linearGradDark.setColorAt(0.93, QColor(0, 105, 0).rgb()); // green
    m_linearGradDark.setColorAt(1, QColor(0, 96, 0).rgb()); // dark green
}

QFont NoiseTrackerPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont NoiseTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

NoiseTrackerPatternView::~NoiseTrackerPatternView()
{
}

void NoiseTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(115, 117, 115);
    QColor colorHilite(173, 170, 173);
    QColor colorShadow(66, 69, 66);
    QColor colorRed(255, 0, 0);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((30) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, (height) - 3, 1, 1, colorBase);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
        painter->fillRect((27) + chan * 72, 32, 1, 1, colorBase);
    }
    painter->fillRect((29) + (m_channels) * 72, 32, 1, 1, colorBase);
    painter->fillRect(0, 32, 1, 1, colorHilite);

    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((3), (height) - 3, 25, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(1, (height) - 1, 319, 1, colorShadow);

    //1px antialias
    painter->fillRect(11, 32, 1, 1, colorBase);
    painter->fillRect(22, 32, 1, 1, colorBase);
    painter->fillRect(45, 32, 1, 1, colorBase);
}

void NoiseTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(115, 117, 115);
    QColor colorHilite(173, 170, 173);
    QColor colorShadow(66, 69, 66);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(left - 2, 0, left - 2, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 0, ((left - 1)), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((left - 0), 0, (left - 0), height - 4);

    //1 pixel bottom antialiasing
    painter->fillRect((left - 1), height - 3, 1, 1, colorBase);
    //1 pixel bottom antialiasing
    painter->fillRect((left - 3), height - 1, 1, 1, colorBase);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 0, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 0, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 0, (left + 27 + chan * 72), (height) - 2);
        //1 pixel top antialiasing
        painter->fillRect((left + 25 + chan * 72), 32, 1, 1, colorBase);
        //1 pixel bottom antialiasing
        painter->fillRect((left + 27 + chan * 72), (height) - 3, 1, 1, colorBase);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(318, 0, 318, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319, 0, 319, height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320, 0, 320, height);
    //1 pixel antialiasing
    painter->fillRect(318, 0, 1, 1, colorBase);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }

        //top hilite
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) - 2 + (topOffset), (71 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) + 10 + (topOffset), (71 + extra), 2, colorShadow);
    }


    //current row left
    //top hilite
    painter->fillRect(((left)), (height / 2) - 2 + (topOffset), 25, 2, colorHilite);
    //bottom shadow
    painter->fillRect(((left - 1)), (height / 2) + 10 + (topOffset), 26, 2, colorShadow);

    //1 pixel hilite antialiasing
    painter->fillRect((left - 1), (height / 2) - 2 + (topOffset), 1, 2, colorBase);

    //main current row
    painter->fillRect(((left - 1)), (height / 2) - 4, 317, 10, colorBase);

    QColor colorGreenHiliteColor(0, 239, 0);
    QColor colorGreenBaseColor(0, 170, 0);
    QColor colorGreenShadowColor(0, 101, 0);
    //vumeters base
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect(((left + 30)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenHiliteColor);
        painter->fillRect(((left + 32)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 6, 1, colorGreenBaseColor);
        painter->fillRect(((left + 38)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenShadowColor);
    }
}
