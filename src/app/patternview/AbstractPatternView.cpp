#include "AbstractPatternView.h"
#include <QApplication>
#include <QDir>
#include "mainwindow.h"
#include "soundmanager.h"

AbstractPatternView::AbstractPatternView(Tracker* parent, unsigned int channels)
{
    m_trackerWindow = parent;
    m_width = 320;
    m_height = 256;
    m_channels = channels;
    m_fontWidth = 8;
    m_fontHeight = 7;
    m_renderTop = false;
    m_renderVUMeter = false;

    effectHex = true;
    effectPad = false;
    parameterHex = true;
    parameterPad = true;
    rowNumberHex = false;
    rowNumberPad = true;
    rowNumberOffset = 1;
    instrumentHex = true;
    instrumentPad = true;
    m_effectsThenParametersEnabled = false;
    volumeHex = false;
    volumePad = true;
    instrumentOffset = 0;
    octaveOffset = 0;
    volumeEnabled = false;
    instrumentEnabled = true;
    effectEnabled = true;
    effect2Enabled = false;
    parameterEnabled = true;
    parameter2Enabled = false;
    m_rowNumbersLastChannelEnabled = false;
    m_RowNumbersEveryChannelEnabled = false;
    m_ColorRowHighlightBackground = "ff00ff";
    m_RowHighlightForegroundFrequency = 0;
    m_RowHighlightBackgroundFrequency = 0;
    m_ColorRowNumberHighLightFrequency = 0;
    m_ColorRowHighlightBackground2 = "00ffff";
    m_RowHighlightBackgroundFrequency2 = 0;
    m_highlightBackgroundOffset = 0;
    m_font = QFont("ImpulseTracker 2");
    m_bitmapFont = BitmapFont("ImpulseTracker 2");
    m_fontWidth = 8;
    m_fontHeight = 8;
    m_font.setPixelSize(6);
    m_font.setStyleStrategy(QFont::NoAntialias);
    m_fontWidthEffects = 0;
    m_fontWidthInstrument = 0;
    m_fontWidthParameters = 0;
    m_fontWidthSeparatorNote = 0;
    m_fontWidthRownumber = 0;
    m_instrumentPaddingCharacter = "0";
    m_xOffsetRow = 0;
    m_xOffsetSeparatorRowNumber = 0;
    m_yOffsetCurrentRowAfter = 0;
    m_yOffsetCurrentRowBefore = 0;
    m_yOffsetRowAfter = 0;
    m_linesBetweenRows = false;
    m_useInstrumentColorOnEffectAndParameterFrequency = 0;

    m_ColorWindowBackground = "000000";
    m_ColorRowNumberBackground = "000000";
    m_ColorRowNumberCurrentRow = m_colorDefault = m_ColorRowNumber = m_ColorRowNumberHighLight = m_ColorInstrument =
        m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty =
        QColor(255, 0, 0);
    m_colorEmptyAlternate = m_colorDefaultAlternate = m_ColorInstrumentAlternate = m_ColorEffectAlternate =
        m_ColorEffect2Alternate = m_ColorParameterAlternate = m_ColorParameter2Alternate = m_ColorVolumeAlternate =
        QColor(255, 255, 0);

    m_ColorRowNumberCurrentRowEnabled = false;
    m_colorDefault = QColor(0, 68, 221);

    m_colorCurrentRowBackground = QColor(119, 119, 119);
    m_colorCurrentRowForeground = QColor(0, 0, 0);
    m_colorCurrentRowHighlightBackground = QColor(119, 119, 119);
    m_colorCurrentRowHighlightForeground = QColor(0, 0, 0);
    m_CurrentRowHighlightBackgroundFrequency = 0;
    m_CurrentRowHighlightForegroundFrequency = 0;
    m_alternateChannelColorsFrequency = 0;

    m_noEmptyInstrumentColor = false;
    m_noEmptyEffectColor = false;
    m_noEmptyParameterColor = false;
    m_noEmptyEffect2Color = false;
    m_noEmptyParameter2Color = false;
    m_noEmptyVolumeColor = false;


    m_SeparatorInstrument = "";
    m_SeparatorVolume = "";
    m_SeparatorChannel = "";
    m_SeparatorRowNumber = "";
    m_SeparatorNote = "";
    m_SeparatorEffect = "";
    m_SeparatorEffect2 = "";
    m_SeparatorParameter = "";
    m_SeparatorParameter2 = "";
    m_SeparatorRowNumberLast = "";
    m_RowStart = "";
    m_RowEnd = "";

    m_emptyParameter = "00";
    m_emptyInstrument = "00";
    m_emptyNote = "---";
    m_emptyEffect = "0";
    m_emptyVolume = "";
    m_empty = "-";
    m_yOffsetRowHighlight = 0;

    m_xChannelStart = 30;
    m_channelWidth = 69;
    m_channelxSpace = 3;
    m_channelLastWidth = -1;
    m_channelFirstWidth = -1;
    m_colorBackground = QColor(0, 0, 0, 180);

    m_iCurrentSample = 0;
    m_ibuttonPrevSampleWidth = 0;
    m_ibuttonPrevSampleHeight = 0;
    m_ibuttonPrevSampleX = 0;
    m_ibuttonPrevSampleY = 0;
    m_ibuttonNextSampleWidth = 0;
    m_ibuttonNextSampleHeight = 0;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 0;


    m_RowLength = 80;

    m_bottomFrameHeight = 3;
    m_topHeight = 33;

    m_vumeterHeight = 47;
    m_vumeterWidth = 10;
    m_vumeterLeftOffset = 55;
    m_vumeterOffset = 72;
    m_vumeterHilightWidth = 2;
    m_vumeterTopOffset = -6;

    m_linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
    m_linearGradHiLite = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
    m_linearGradDark = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
}

const char* AbstractPatternView::NOTES[109] =
{
    "", "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
    "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
    "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
    "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
    "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
    "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};


AbstractPatternView::~AbstractPatternView()
{
}


void AbstractPatternView::setupVUMeters()
{
    m_linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
    m_linearGradHiLite = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
    m_linearGradDark = QLinearGradient(QPointF(0, 0), QPointF(0, m_vumeterHeight));
}

QString AbstractPatternView::note(BaseRow* row)
{
    int note = row->note;
    if (note != 0)
    {
        note -= octaveOffset;
    }
    if (note >= 109 || note < 0)
    {
        note = 0;
    }
    if (note == 0)
    {
        return m_emptyNote;
    }
    else
    {
        return NOTES[note];
    }
}

QFont AbstractPatternView::font()
{
    return m_font;
}

BitmapFont AbstractPatternView::bitmapFont()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::currentRowBitmapFont()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::infoFont()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::infoFont2()
{
    return m_bitmapFont;
}

QFont AbstractPatternView::currentRowFont()
{
    return m_font;
}

BitmapFont AbstractPatternView::bitmapFontEffects()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::bitmapFontParameters()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::bitmapFontInstrument()
{
    return m_bitmapFont;
}

BitmapFont AbstractPatternView::bitmapFontRownumber()
{
    return m_bitmapFont;
}

QFont AbstractPatternView::fontEffects()
{
    return m_font;
}

QFont AbstractPatternView::fontParameters()
{
    return m_font;
}

QFont AbstractPatternView::fontInstrument()
{
    return m_font;
}

QFont AbstractPatternView::fontRownumber()
{
    return m_font;
}


void AbstractPatternView::paintAbove(__attribute__((unused)) QPainter* painter, __attribute__((unused)) int height,
                                     __attribute__((unused)) int currentRow)
{
}

void AbstractPatternView::paintBelow(__attribute__((unused)) QPainter* painter, __attribute__((unused)) int height,
                                     __attribute__((unused)) int currentRow)
{
}

void AbstractPatternView::paintTop(QPainter* painter, Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{

}
void AbstractPatternView::drawVUMeters(QPainter* painter)
{
    int height = m_height;
    int width = m_width;

    Tracker* t = (Tracker*)this->parent();
    SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);

    int HEIGHT = m_vumeterHeight;
    int WIDTH = m_vumeterWidth;
    int LEFT_OFFSET = m_vumeterLeftOffset;
    int VUMETER_OFFSET = m_vumeterOffset;
    int HILIGHT_WIDTH = m_vumeterHilightWidth;
    int TOP_OFFSET = m_vumeterTopOffset;
    int maxHeight = HEIGHT;


    if (QString(t->m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0") ||
        QString(t->m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1") ||
        QString(t->m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
    {
        painter->translate(0, (height) - maxHeight + TOP_OFFSET);
    }
    else
    {
        painter->translate(0, (height / 2) - maxHeight + TOP_OFFSET);
    }

    for (int unsigned i = 0; i < t->m_info->numChannels; i++)
    {
        unsigned char volume = static_cast<unsigned char>(((t->m_info->modVUMeters[i] / 64.0) + 0.005) * 100);
        //volume is between 0-100. 0.005 for rounding

        int vumeterCurrentHeight = (volume * HEIGHT / 100);

        int vuWidth = WIDTH;
        int xPos = (i * VUMETER_OFFSET) + LEFT_OFFSET;


        m_rectL = QRect(xPos, maxHeight - vumeterCurrentHeight, vuWidth, vumeterCurrentHeight);
        m_rectLHiLite = QRect(xPos, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH, vumeterCurrentHeight);
        m_rectLDark = QRect(xPos + vuWidth - HILIGHT_WIDTH, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH,
                        vumeterCurrentHeight);

        painter->fillRect(m_rectL, QBrush(m_linearGrad));
        painter->fillRect(m_rectLHiLite, QBrush(m_linearGradHiLite));
        painter->fillRect(m_rectLDark, QBrush(m_linearGradDark));

    }


}
void AbstractPatternView::drawText(QString text, QPainter* painter, int numPixels, int yPixelPosition, BitmapFont font, int letterSpacing)
{
    if (yPixelPosition >height() || numPixels > painter->window().width() || yPixelPosition <
                                                                                       0 || yPixelPosition > painter->window().height())
    {
        return;
    }
    //Hack for Hivelytracker which uses a true type font
    if ((m_font.family() != "DejaVu Sans Mono"))
    {
        int x = numPixels;
        for (int i = 0; i < text.length(); i++)
        {
            if (font.m_characterWidths.isEmpty())
            {
                x = numPixels + (font.m_fontWidth * i);
                if (letterSpacing != 0)
                {
                    x += letterSpacing * i;
                }
                int y = yPixelPosition - (font.m_fontHeight);
                painter->drawPixmap(x, y, font.m_fontWidth, font.m_fontHeight, font.m_characterMap,
                                    font.m_characterPositions.value(text.at(i)).x(),
                                    font.m_characterPositions.value(text.at(i)).y(), font.m_fontWidth,
                                    font.m_fontHeight);
            }
                //variable width font
            else
            {
                if (letterSpacing != 0)
                {
                    x += letterSpacing * i;
                }
                int y = yPixelPosition - (font.m_fontHeight);
                painter->drawPixmap(x, y, font.m_characterWidths.value(text.at(i)), font.m_fontHeight,
                                    font.m_characterMap, font.m_characterPositions.value(text.at(i)).x(),
                                    font.m_characterPositions.value(text.at(i)).y(),
                                    font.m_characterWidths.value(text.at(i)), font.m_fontHeight);
                x += font.m_characterWidths.value(text.at(i));
            }
        }
    }
    else
    {
        painter->setFont(m_font);
        painter->drawText(numPixels, yPixelPosition, text);
    }
}
/* check which channel x, y is */
int AbstractPatternView::getChannelClicked(int x, int y)
{
    //TODO r�kna med firstchannelwidth annars blir all lite off efter f�rsta chan, tex multitracker
    //TODO r�knas med sista s� att g�r klicka p� hela, tex. octamed 5
    if (x < m_xChannelStart)
    {
        return -1;
    }
    return (x - m_xChannelStart) / ((m_channelWidth + m_channelxSpace));
}

QString AbstractPatternView::effect(BaseRow* row)
{
    if (!effectEnabled) return "";
    int effect = row->effect;
    if (effect == 0) return m_emptyEffect;
    int base = effectHex ? 16 : 10;

    QString effectStr = QString::number(effect, base).toUpper();
    effectStr = effectPad && effectStr.length() == 1 ? "0" + effectStr : effectStr;
    return effectStr;
}

QString AbstractPatternView::parameter(BaseRow* row)
{
    if (!parameterEnabled) return "";
    int parameter = row->param;
    if (parameter == 0) return m_emptyParameter;
    int base = parameterHex ? 16 : 10;

    QString parameterStr = QString::number(parameter, base).toUpper();
    parameterStr = parameterPad && parameterStr.length() == 1 ? "0" + parameterStr : parameterStr;
    return parameterStr;
}

QString AbstractPatternView::effect2(BaseRow* row)
{
    if (!effect2Enabled) return "";
    int effect2 = row->effect2;
    if (effect2 == 0) return m_emptyEffect;
    int base = effectHex ? 16 : 10;

    QString effectStr = QString::number(effect2, base).toUpper();
    effectStr = effectPad && effectStr.length() == 1 ? "0" + effectStr : effectStr;
    return effectStr;
}

QString AbstractPatternView::parameter2(BaseRow* row)
{
    if (!parameter2Enabled) return "";
    int parameter2 = row->param2;
    if (parameter2 == 0) return m_emptyParameter;
    int base = parameterHex ? 16 : 10;

    QString parameterStr = QString::number(parameter2, base).toUpper();
    parameterStr = parameterPad && parameterStr.length() == 1 ? "0" + parameterStr : parameterStr;
    return parameterStr;
}

QString AbstractPatternView::volume(BaseRow* row)
{
    if (!volumeEnabled) return "";
    int volume = row->vol;
    if (volume < -1) volume = -1;
    if (volume == -1) return m_emptyVolume;
    int base = volumeHex ? 16 : 10;

    QString volumeStr = QString::number(volume, base).toUpper();
    volumeStr = volumePad && volumeStr.length() == 1 ? "0" + volumeStr : volumeStr;
    return volumeStr;
}

QString AbstractPatternView::instrument(BaseRow* row)
{
    if (!instrumentEnabled) return "";
    int instrument = row->sample;
    if (instrument == 0) return m_emptyInstrument;
    int base = instrumentHex ? 16 : 10;
    instrument += instrumentOffset;
    if (instrument < 0) instrument = 0;
    QString instrumentStr = QString::number(instrument, base).toUpper();
    instrumentStr = instrumentPad && instrumentStr.length() == 1
                        ? m_instrumentPaddingCharacter + instrumentStr
                        : instrumentStr;
    return instrumentStr;
}

QString AbstractPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber + rowNumberOffset, base).toUpper();
    rowNumberStr = rowNumberPad && rowNumberStr.length() == 1 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}
void AbstractPatternView::drawVerticalEmboss(int xPos, int yPos, int height, QColor hilite, QColor shadow, QColor base,
                                 QPainter* painter, bool left, bool right)
{
    if (left)
    {
        painter->fillRect(xPos, (1 + yPos), 1, (height - 1), shadow);
        painter->fillRect(xPos, (yPos), 1, 1, base);
    }
    if (right)
    {
        painter->fillRect(xPos + (1), (yPos), 1, height, hilite);
        painter->fillRect(xPos + (1), (yPos + height), 1, 1, base);
    }
}
std::vector<bool> AbstractPatternView::getChannelMuteMask()
{
    std::vector<bool> muteMask;

    Tracker* t = dynamic_cast<Tracker*>(this->parent());
    if (!t || !t->m_info)
        return muteMask;

    muteMask.resize(t->m_info->numChannels);

    for (unsigned int i = 0; i < t->m_info->numChannels; i++)
    {
        muteMask[i] = SoundManager::getInstance().isChannelMuted(i);
    }

    return muteMask;
}
void AbstractPatternView::updateEnabledChannels(QPainter* painter)
{
    Tracker* t = (Tracker*)this->parent();
    for (int unsigned i = 0; i < t->m_info->numChannels; i++)
    {
        if (SoundManager::getInstance().isChannelMuted(i))
        {
            //draw background with opacity when enabled/disabled channels
            QBrush brush(m_colorBackground);
            int x = m_xChannelStart + (i * m_channelWidth) + (i * m_channelxSpace);
            int y = m_topHeight;
            int w;
            if (i == 0) //first channel
            {
                w = channelFirstWidth();
            }
            else if (i == t->m_info->numChannels - 1) //last channel
            {
                if (channelFirstWidth() != m_channelWidth)
                {
                    x = x - (m_channelWidth - channelFirstWidth());
                }
                w = channelLastWidth();
            }
            else
            {
                if (channelFirstWidth() != m_channelWidth)
                {
                    x = x - (m_channelWidth - channelFirstWidth());
                }
                w = m_channelWidth;
            }

            int h = m_height - y - m_bottomFrameHeight;
            painter->fillRect(QRect(x, y, w, h), brush);
        }
    }
}