#include "tracker.h"
#include <plugins.h>
#include "mainwindow.h"
#include "qevent.h"
#include <QDir>
#include <QApplication>
#include "soundmanager.h"
#include "patternview/ProTracker1PatternView.h"
#include "patternview/SoundFXPatternView.h"
#include "patternview/ScreamTracker3PatternView.h"
#include "patternview/ScreamTracker2PatternView.h"
#include "patternview/UltimateSoundTrackerPatternView.h"
#include "patternview/Composer669PatternView.h"
#include "patternview/HivelyTrackerPatternView.h"
#include "patternview/AHXPatternView.h"
#include "patternview/MultiTrackerPatternView.h"
#include "patternview/DigiBooster17PatternView.h"
#include "patternview/StarTrekker13PatternView.h"
#include "patternview/UltraTrackerPatternView.h"
#include "patternview/GameMusicCreatorPatternView.h"
#include "patternview/IceTrackerPatternView.h"
#include "patternview/ChipTrackerPatternView.h"
#include "patternview/ImpulseTrackerPatternView.h"
#include "patternview/OktalyzerPatternView.h"
#include "patternview/FastTracker2PatternView.h"
#include "patternview/FastTracker1PatternView.h"
#include "patternview/MEDPatternView.h"
#include "patternview/DigiBoosterProPatternView.h"
#include "patternview/OctaMEDPatternView.h"
#include "patternview/FastTracker24ChanPatternView.h"
#include "patternview/FastTracker26ChanPatternView.h"
#include "patternview/NoiseTrackerPatternView.h"
#include "patternview/SoundTracker26PatternView.h"
#include "patternview/OctaMED44ChanPatternView.h"
#include "patternview/OctaMED54ChanPatternView.h"
#include "patternview/OctaMED5ChanPatternView.h"
#include "patternview/OctaMEDSoundstudioPatternView.h"
#include "patternview/ProTracker36PatternView.h"
#include "patternview/GenericPatternView.h"
#include <QFontDatabase>

Tracker::Tracker()
{
    QString fontDir = QApplication::applicationDirPath() + QDir::separator() + "data/resources/trackerview/fonts" +
        QDir::separator();
    QFontDatabase::addApplicationFont(fontDir + "DejaVuSansMono.ttf");
}

void Tracker::init()
{
    m_info = SoundManager::getInstance().m_Info1;
    m_render = false;
    m_renderVUMeter = false;
    m_renderTop = false;
    m_fColorHueCounter = 0;
    m_fColorLightnessCounter = 0;
    m_fColorLightnessdirection = 0.3f;
    m_fColorSaturationCounter = 0;
    m_fColorSaturationdirection = 0.75f;
    if (m_info == nullptr)
    {
        return;
    }

    if (m_info == nullptr)
    {
        m_trackerview = 0;
        m_render = false;
    }

    if (m_info->plugin != PLUGIN_libxmp && m_info->plugin != PLUGIN_hivelytracker && m_info->plugin != PLUGIN_libopenmpt
        && m_info->plugin != PLUGIN_sunvox)
    {
        m_trackerview = 0;
        m_render = false;
        return;
    }
    m_render = true;

    if (false)
    {
        m_trackerview = new GenericPatternView(this, m_info->numChannels, m_cSFXFontSize);
        m_renderVUMeter = false;
        m_renderTop = false;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (fa08)"))
    {
        m_trackerview = 0;
        m_render = false;
        return;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (cd81)"))
    {
        m_trackerview = 0;
        m_render = false;
        return;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("SunVox"))
    {
        m_trackerview = new SoundFXPatternView(this, m_info->numChannels, m_cSFXFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (6chn") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (8chn") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (16ch"))
    {
        m_trackerview = new FastTracker1PatternView(this, m_info->numChannels, m_cFasttracker1FontSize);
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("soundfx"))
    {
        m_trackerview = new SoundFXPatternView(this, m_info->numChannels, m_cSFXFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("ice tracker"))
    {
        m_trackerview = new IceTrackerPatternView(this, m_info->numChannels, m_cIceTrackerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (patt"))
    {
        m_renderTop = true;
        m_renderVUMeter = true;
        m_trackerview = new ProTracker36PatternView(this, m_info->numChannels, m_cProTracker36FontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (flt4"))
    {
        m_trackerview = new StarTrekker13PatternView(this, m_info->numChannels, m_cStarTrekkerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }
    else if ((QString(m_info->fileformat.c_str()).toLower().startsWith("protracker") && !
            QString(m_info->fileformat.c_str()).toLower().startsWith("protracker xm")) ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("probably converted (m.k") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("unknown/converted (m.k") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("converted 15 ins") || QString(
            m_info->fileformat.c_str()).
        toLower().startsWith("unknown or converted (M"))
    {
        m_trackerview = new ProTracker1PatternView(this, m_info->numChannels, m_cProTrackerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
        m_height = 32;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("noise") || QString(m_info->fileformat.c_str()).
        toLower().startsWith("his master"))
    {
        m_trackerview = new NoiseTrackerPatternView(this, m_info->numChannels, m_cProTrackerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("mnemotron"))
    {
        m_trackerview = new SoundTracker26PatternView(this, m_info->numChannels, m_cSoundTracker26FontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }

    else if (QString(m_info->fileformat.c_str()).toLower() == "soundtracker")
    {
        m_trackerview = new UltimateSoundTrackerPatternView(this, m_info->numChannels, m_cUltimateSoundTrackerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }


    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("game music creator"))
    {
        m_trackerview = new GameMusicCreatorPatternView(this, m_info->numChannels, m_cGMCFontSize);
        m_renderVUMeter = false;
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()) == "AHX")
    {
        m_trackerview = new AHXPatternView(this, m_info->numChannels, m_cAHXFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("chiptracker"))
    {
        m_trackerview = new ChipTrackerPatternView(this, m_info->numChannels, m_cChipTrackerFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }

    else if (QString(m_info->fileformat.c_str()) == "HivelyTracker")
    {
        m_trackerview = new HivelyTrackerPatternView(this, m_info->numChannels, m_cHivelyFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }

    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("scream tracker 3"))
    {
        m_trackerview = new ScreamTracker3PatternView(this, m_info->numChannels, m_cScreamTracker3FontSize);
    }

    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("scream tracker 2"))
    {
        m_trackerview = new ScreamTracker2PatternView(this, m_info->numChannels, m_cScreamTracker2FontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("fasttracker 2") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("skale tracker xm") ||
        QString(m_info->fileformat.c_str()).toLower().startsWith("madtracker 2.0 xm") ||
        QString(m_info->fileformat.c_str()).toLower().endsWith("xm 1.04"))
    {
        m_renderTop = true;
        if (m_info->numChannels > 6)
        {
            m_trackerview = new FastTracker2PatternView(this, m_info->numChannels, m_cFasttracker2FontSize);
        }
        else if (m_info->numChannels > 4)
        {
            m_trackerview = new FastTracker26ChanPatternView(this, m_info->numChannels, m_cFasttracker2FontSize);
        }
        else
        {
            m_trackerview = new FastTracker24ChanPatternView(this, m_info->numChannels, m_cFasttracker2FontSize);
        }
    }

    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0"))
    {
        m_trackerview = new MEDPatternView(this, m_info->numChannels, m_cMEDFontSize);
        m_renderVUMeter = true;
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("impulse"))
    {
        m_trackerview = new ImpulseTrackerPatternView(this, m_info->numChannels, m_cImpulseTracker2FontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("composer 669"))
    {
        m_trackerview = new Composer669PatternView(this, m_info->numChannels, m_cComposer669FontSize);
        m_height = 16;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("ultratracker"))
    {
        m_trackerview = new UltraTrackerPatternView(this, m_info->numChannels, m_cUltraTrackerFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("digibooster pro"))
    {
        m_trackerview = new DigiBoosterProPatternView(this, m_info->numChannels, m_cDigiBoosterProFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("digibooster"))
    {
        m_trackerview = new DigiBooster17PatternView(this, m_info->numChannels, m_cDigiBoosterFontSize);
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("oktalyzer"))
    {
        m_trackerview = new OktalyzerPatternView(this, m_info->numChannels, m_cOktalyzerFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1"))
    {
        m_renderTop = true;
        if (m_info->numChannels > 4)
        {
            m_trackerview = new OctaMEDPatternView(this, m_info->numChannels, m_cMEDFontSize);
        }
        else
        {
            m_trackerview = new OctaMED44ChanPatternView(this, m_info->numChannels, m_cMEDFontSize);
            m_renderVUMeter = true;
        }
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
    {
        if (m_info->numChannels > 4)
        {
            m_trackerview = new OctaMED5ChanPatternView(this, m_info->numChannels, m_cMEDFontSize);
            m_renderTop = true;
        }
        else
        {
            m_trackerview = new OctaMED54ChanPatternView(this, m_info->numChannels, m_cMEDFontSize);
            m_renderVUMeter = true;
            m_renderTop = true;
        }
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd3"))
    {
        m_trackerview = new OctaMEDSoundstudioPatternView(this, m_info->numChannels, m_cOctaMEDSSFontSize);
        m_renderTop = true;
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("multitracker"))
    {
        m_renderTop = true;
        m_trackerview = new MultiTrackerPatternView(this, m_info->numChannels, m_cMultiTrackerFontSize);
    }
    else if (QString(m_info->fileformat.c_str()).toLower().startsWith("mdx"))
    {
        m_trackerview = new MultiTrackerPatternView(this, m_info->numChannels, m_cMultiTrackerFontSize);
    }
    else
    {
        m_trackerview = 0;
        m_render = false;
        return;
    }


    m_currentTrackPositions = std::vector<unsigned char>();
    m_currentTrackPositions.clear();
    m_currentRow = 0;
    m_currentPattern = -1;
    m_currentPosition = -1;
    m_currentSpeed = -1;
    m_currentBPM = -1;

    if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_hivelytracker || m_info->plugin == PLUGIN_libxmp
        || m_info->plugin == PLUGIN_sunvox)
    {
        SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN_INFO);
    }
}

void Tracker::setFont(BitmapFont font)
{
    m_font = font;
}

void Tracker::drawText(QString text, QPainter* painter, int numPixels, int yPixelPosition, int letterSpacing)
{
    if (yPixelPosition > m_trackerview->height() || numPixels > painter->window().width() / m_scale || yPixelPosition <
        0 || yPixelPosition > painter->window().height() / m_scale)
    {
        return;
    }
    //Hack for Hivelytracker which uses a true type font
    if ((m_trackerview->font().family() != "DejaVu Sans Mono"))
    {
        int x = numPixels;
        for (int i = 0; i < text.length(); i++)
        {
            if (m_font.m_characterWidths.isEmpty())
            {
                x = numPixels + (m_font.m_fontWidth * i);
                if (letterSpacing != 0)
                {
                    x += letterSpacing * i;
                }
                int y = yPixelPosition - (m_font.m_fontHeight);
                painter->drawPixmap(x, y, m_font.m_fontWidth, m_font.m_fontHeight, m_font.m_characterMap,
                                    m_font.m_characterPositions.value(text.at(i)).x(),
                                    m_font.m_characterPositions.value(text.at(i)).y(), m_font.m_fontWidth,
                                    m_font.m_fontHeight);
            }
            //variable width font
            else
            {
                if (letterSpacing != 0)
                {
                    x += letterSpacing * i;
                }
                int y = yPixelPosition - (m_font.m_fontHeight);
                painter->drawPixmap(x, y, m_font.m_characterWidths.value(text.at(i)), m_font.m_fontHeight,
                                    m_font.m_characterMap, m_font.m_characterPositions.value(text.at(i)).x(),
                                    m_font.m_characterPositions.value(text.at(i)).y(),
                                    m_font.m_characterWidths.value(text.at(i)), m_font.m_fontHeight);
                x += m_font.m_characterWidths.value(text.at(i));
            }
        }
    }
    else
    {
        painter->setFont(m_trackerview->font());
        painter->drawText(numPixels, yPixelPosition, text);
    }
}


void Tracker::paint(QPainter* painter, QPaintEvent* event)
{
    if (!m_render)
    {
        return;
    }
    int height = m_trackerview->height();
    int width = m_trackerview->width();
    float scaleX = float(event->rect().width()) / float(m_trackerview->width());
    float scaleY = float(event->rect().height()) / float(m_trackerview->height());

    m_scale = min(scaleX, scaleY);
    painter->scale(m_scale, m_scale);

    //painter->setRenderHint(QPainter::Antialiasing);
    //painter->setRenderHint(QPainter::SmoothPixmapTransform);


    setFont(m_trackerview->bitmapFont());


    unsigned int currentPattern = 0;
    unsigned int currentRow = 0;
    unsigned int currentPosition = 0;
    unsigned int currentSpeed = 0;
    unsigned int currentBPM = 0;

    currentPosition = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODORDER);
    currentRow = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODROW);
    currentPattern = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN);
    currentSpeed = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_SPEED);
    currentBPM = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_BPM);

    bool updateHively = false;
    if (m_info->plugin == PLUGIN_hivelytracker)
    {
        for (unsigned int v = 0; v < m_info->modTrackPositions.size(); v++)
        {
            if (m_currentTrackPositions.empty() || m_currentTrackPositions.size() != m_info->modTrackPositions.size())
            {
                updateHively = true;
                break;
            }
            if (m_currentTrackPositions[v] != m_info->modTrackPositions[v])
            {
                updateHively = true;
                break;
            }
        }
    }


    if (true)
    //if(currentRow!=m_currentRow || m_renderVUMeter || updateHively) //only redraw if it's a new row
    {
        //Revert painter scaling and fill background rect
        QRect rect(0, 0, painter->window().width() / m_scale, painter->window().height() / m_scale);
        painter->fillRect(rect, QColor(0, 0, 0));
        //cout << "painter->window().width()*m_scale " << painter->window().width()*m_scale << "\n";
        //cout << "m_scale " << m_scale << "\n";
        //cout << "painter->window().width() " << painter->window().width() << "\n";


        m_currentRow = currentRow;
        m_currentPattern = currentPattern; //used where one pattern per positions (mod/sfx)
        m_currentPosition = currentPosition;
        m_currentSpeed = currentSpeed;
        m_currentBPM = currentBPM;

        if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_libxmp)
        {
            m_info->modPatternRows = m_info->patterns[m_currentPattern].size() / m_info->numChannels;
        }

        if (m_info->plugin == PLUGIN_hivelytracker)
        {
            m_currentTrackPositions = std::vector<unsigned char>(m_info->modTrackPositions.size());
            copy(m_info->modTrackPositions.begin(), m_info->modTrackPositions.end(), m_currentTrackPositions.begin());
            //used where multiple patterns positions per position (ahx)
        }

        unsigned int j = 0;
        unsigned int i = 0;
        unsigned int outerLoopBreakValue = 0;
        if (m_info->plugin == PLUGIN_hivelytracker)
        {
            outerLoopBreakValue = m_info->modPatternRows;
        }

        else if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_libxmp)
        {
            i = 0;
            outerLoopBreakValue = m_info->patterns[m_currentPattern].size();
        }


        m_trackerview->paintBelow(painter, height, currentRow);


        int libxmpNotes = 0;
        int yPixelPosition = 0;
        while (i < outerLoopBreakValue)
        {
            bool insideVisibleArea;

            QString row = m_trackerview->rowNumber(j);

            QString strNote;
            QString strRowNumber;
            QColor colorRowNumber = m_trackerview->colorRowNumber();
            QColor colorInstrument = m_trackerview->colorInstrument();
            QColor colorEffect = m_trackerview->colorEffect();
            QColor colorEffect2 = m_trackerview->colorEffect2();
            QColor colorVolume = m_trackerview->colorVolume();
            QColor colorParameter = m_trackerview->colorParameter();
            QColor colorParameter2 = m_trackerview->colorParameter2();
            QColor colorDefault = m_trackerview->colorDefault();

            QColor colorInstrumentAlt;
            QColor colorEffectAlt;
            QColor colorEffect2Alt;
            QColor colorVolumeAlt;
            QColor colorParameterAlt;
            QColor colorParameter2Alt;
            QColor colorDefaultAlt;
            QColor colorEmptyAlt;


            if (m_trackerview->rowHighlightForegroundFrequency())
            {
                if (!(j % m_trackerview->rowHighlightForegroundFrequency()))
                {
                    colorRowNumber = m_trackerview->colorRowHighlightForeground();
                    colorInstrument = m_trackerview->colorRowHighlightForeground();
                    colorEffect = m_trackerview->colorRowHighlightForeground();
                    colorEffect2 = m_trackerview->colorRowHighlightForeground();
                    colorVolume = m_trackerview->colorRowHighlightForeground();
                    colorParameter = m_trackerview->colorRowHighlightForeground();
                    colorParameter2 = m_trackerview->colorRowHighlightForeground();
                    colorDefault = m_trackerview->colorRowHighlightForeground();
                }
            }

            if (m_trackerview->colorRowNumberHighLightFrequency())
            {
                if (!(j % m_trackerview->colorRowNumberHighLightFrequency()))
                {
                    colorRowNumber = m_trackerview->colorRowNumberHighLight();
                }
            }

            strRowNumber = row + m_trackerview->separatorRowNumber();

            if (j == currentRow)
            {
                yPixelPosition += (m_trackerview->currentRowFont().pixelSize() + m_trackerview->yOffsetRowAfter());
                yPixelPosition -= m_trackerview->yOffsetCurrentRowBefore();
                if (currentRow == 0)
                {
                    yPixelPosition += m_trackerview->yOffsetCurrentRowBefore();
                    yPixelPosition -= (m_trackerview->fontHeight() + m_trackerview->yOffsetRowAfter());
                }
                setFont(m_trackerview->currentRowBitmapFont());
                painter->setPen(m_trackerview->colorCurrentRowForeground());

                if (!m_trackerview->colorRowNumberCurrentRowEnabled())
                {
                    colorRowNumber = m_trackerview->colorCurrentRowForeground();
                }
                else
                {
                    colorRowNumber = m_trackerview->colorRowNumberCurrentRow();
                }
                colorInstrument = m_trackerview->colorCurrentRowForeground();
                colorEffect = m_trackerview->colorCurrentRowForeground();
                colorEffect2 = m_trackerview->colorCurrentRowForeground();
                colorVolume = m_trackerview->colorCurrentRowForeground();
                colorParameter = m_trackerview->colorCurrentRowForeground();
                colorParameter2 = m_trackerview->colorCurrentRowForeground();
                colorDefault = m_trackerview->colorCurrentRowForeground();

                colorInstrumentAlt = m_trackerview->colorCurrentRowForeground();
                colorEffectAlt = m_trackerview->colorCurrentRowForeground();
                colorEffect2Alt = m_trackerview->colorCurrentRowForeground();
                colorVolumeAlt = m_trackerview->colorCurrentRowForeground();
                colorParameterAlt = m_trackerview->colorCurrentRowForeground();
                colorParameter2Alt = m_trackerview->colorCurrentRowForeground();
                colorDefaultAlt = m_trackerview->colorCurrentRowForeground();
                if (m_trackerview->currentRowHighlightForegroundFrequency())
                {
                    if (!(j % m_trackerview->currentRowHighlightForegroundFrequency()))
                    {
                        colorRowNumber = m_trackerview->colorCurrentRowHighlightForeground();
                        colorInstrument = m_trackerview->colorCurrentRowHighlightForeground();
                        colorEffect = m_trackerview->colorCurrentRowHighlightForeground();
                        colorEffect2 = m_trackerview->colorCurrentRowHighlightForeground();
                        colorVolume = m_trackerview->colorCurrentRowHighlightForeground();
                        colorParameter = m_trackerview->colorCurrentRowHighlightForeground();
                        colorParameter2 = m_trackerview->colorCurrentRowHighlightForeground();
                        colorDefault = m_trackerview->colorCurrentRowHighlightForeground();
                    }
                }
            }
            else
            {
                yPixelPosition = ((m_trackerview->fontHeight() + m_trackerview->yOffsetRowAfter()) * j) - (currentRow *
                    (m_trackerview->font().pixelSize() + m_trackerview->yOffsetRowAfter()));

                if (j > currentRow && m_trackerview->currentRowFont().pixelSize() > m_trackerview->font().pixelSize())
                {
                    yPixelPosition += m_trackerview->fontHeight() + m_trackerview->yOffsetRowAfter();
                }
                if (j > currentRow)
                {
                    yPixelPosition += m_trackerview->yOffsetCurrentRowAfter();
                }
                if (j < currentRow)
                {
                    yPixelPosition += m_trackerview->yOffsetCurrentRowBefore();
                }

                setFont(m_trackerview->bitmapFont());
                painter->setPen(colorDefault);
            }


            int fontWidth = m_trackerview->fontWidth();

            int numPixels = 0;


            if (!m_trackerview->rowStart().isEmpty())
            {
                painter->setPen(colorRowNumber);
                drawText(m_trackerview->rowStart(), painter, m_trackerview->xOffsetRow() + numPixels,
                         height / 2 + yPixelPosition);
            }
            numPixels += (m_trackerview->rowStart().length() * fontWidth);


            if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->bitmapFontRownumber().
                m_bitmapFontPath)
            {
                setFont(m_trackerview->currentRowBitmapFont());
                fontWidth = m_trackerview->fontWidth();
            }
            else
            {
                setFont(m_trackerview->bitmapFontRownumber());
                fontWidth = m_trackerview->fontWidthRownumber();
            }
            painter->setPen(colorRowNumber);


            drawText(strRowNumber, painter, (m_trackerview->xOffsetRow() + numPixels), height / 2 + yPixelPosition);


            numPixels += (strRowNumber.length() * fontWidth) + m_trackerview->xOffsetSeparatorRowNumber();

            for (int chan = 0; chan < m_info->numChannels; chan++)
            {
                bool alternateChannelColors = false;
                if (m_trackerview->alternateChannelColorsFrequency())
                {
                    if (!(chan % m_trackerview->alternateChannelColorsFrequency()) && j != currentRow)
                    {
                        alternateChannelColors = true;
                        colorInstrumentAlt = m_trackerview->colorInstrumentAlternate();
                        colorEffectAlt = m_trackerview->colorEffectAlternate();
                        colorEffect2Alt = m_trackerview->colorEffect2Alternate();
                        colorVolumeAlt = m_trackerview->colorVolumeAlternate();
                        colorParameterAlt = m_trackerview->colorParameterAlternate();
                        colorParameter2Alt = m_trackerview->colorParameter2Alternate();
                        colorDefaultAlt = m_trackerview->colorDefaultAlternate();
                        colorEmptyAlt = m_trackerview->colorEmptyAlternate();
                    }
                }
                fontWidth = m_trackerview->fontWidth();
                int k = 0;
                if (m_info->plugin == PLUGIN_hivelytracker)
                {
                    k = m_info->modTrackPositions.at(chan) * m_info->modPatternRows;
                }
                else if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_libxmp || m_info->plugin ==
                    PLUGIN_sunvox)
                {
                    k = chan;
                }

                BaseRow* row;
                if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_libxmp || m_info->plugin ==
                    PLUGIN_sunvox)
                {
                    row = m_info->patterns[m_currentPattern][libxmpNotes];
                    libxmpNotes++;
                }
                else
                {
                    row = m_info->modRows[i + k];
                }


                QString instrument = m_trackerview->instrument(row);
                QString effect = m_trackerview->effect(row);
                QString parameter = m_trackerview->parameter(row);
                QString effect2 = m_trackerview->effect2(row);
                QString parameter2 = m_trackerview->parameter2(row);
                strNote = m_trackerview->note(row);
                QString volume = m_trackerview->volume(row);
                QString channelSeparator = m_trackerview->separatorChannel();
                if (chan == 0)
                {
                    channelSeparator = "";
                }
                QString strRowNumberMultiple = strRowNumber;
                if (!m_trackerview->rowNumbersEveryChannelEnabled() || chan == 0)
                {
                    strRowNumberMultiple = "";
                }

                if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                    bitmapFontRownumber().m_bitmapFontPath)
                {
                    setFont(m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    setFont(m_trackerview->bitmapFontRownumber());
                    fontWidth = m_trackerview->fontWidthRownumber();
                }

                drawText(channelSeparator, painter, (m_trackerview->xOffsetRow() + numPixels),
                         height / 2 + yPixelPosition);
                numPixels += channelSeparator.length() * fontWidth;
                drawText(strRowNumberMultiple, painter, (m_trackerview->xOffsetRow() + numPixels),
                         height / 2 + yPixelPosition);
                numPixels += strRowNumberMultiple.length() * fontWidth;
                if (j == currentRow)
                {
                    setFont(m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    setFont(m_trackerview->bitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                if (strNote == m_trackerview->emptyNote() && j != currentRow)
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(m_trackerview->colorEmpty());
                    }
                    else
                    {
                        painter->setPen(colorEmptyAlt);
                    }
                }
                else
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(colorDefault);
                    }
                    else
                    {
                        painter->setPen(colorDefaultAlt);
                    }
                }

                drawText(strNote, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);

                numPixels += strNote.length() * fontWidth;
                fontWidth = m_trackerview->fontWidthSeparatorNote();
                drawText(m_trackerview->separatorNote(), painter, m_trackerview->xOffsetRow() + numPixels,
                         height / 2 + yPixelPosition);

                numPixels += m_trackerview->separatorNote().length() * fontWidth;
                if (instrument == m_trackerview->emptyInstrument() && !m_trackerview->noEmptyInstrumentColor() && j !=
                    currentRow)
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(m_trackerview->colorEmpty());
                    }
                    else
                    {
                        painter->setPen(colorEmptyAlt);
                    }
                }
                else
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(colorInstrument);
                    }
                    else
                    {
                        painter->setPen(colorInstrumentAlt);
                    }
                }

                if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                    bitmapFontInstrument().m_bitmapFontPath)
                {
                    setFont(m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    setFont(m_trackerview->bitmapFontInstrument());
                    fontWidth = m_trackerview->fontWidthInstrument();
                }

                drawText(instrument, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                numPixels += instrument.length() * fontWidth;
                drawText(m_trackerview->separatorInstrument(), painter, m_trackerview->xOffsetRow() + numPixels,
                         height / 2 + yPixelPosition);
                numPixels += m_trackerview->separatorInstrument().length() * fontWidth;

                if (volume == m_trackerview->emptyVolume() && !m_trackerview->noEmptyVolumeColor() && j != currentRow)
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(m_trackerview->colorEmpty());
                    }
                    else
                    {
                        painter->setPen(colorEmptyAlt);
                    }
                }
                else
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(colorVolume);
                    }
                    else
                    {
                        painter->setPen(colorEmptyAlt);
                    }
                }
                drawText(volume, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                numPixels += volume.length() * fontWidth;
                drawText(m_trackerview->separatorVolume(), painter, m_trackerview->xOffsetRow() + numPixels,
                         height / 2 + yPixelPosition);
                numPixels += m_trackerview->separatorVolume().length() * fontWidth;

                if (!m_trackerview->effectsThenParametersEnabled())
                {
                    if (effect == m_trackerview->emptyEffect() && !m_trackerview->noEmptyEffectColor() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            if (m_trackerview->useInstrumentColorOnEffectAndParameterFrequency() && j % m_trackerview->
                                useInstrumentColorOnEffectAndParameterFrequency() == 0)
                            {
                                painter->setPen(colorInstrument);
                            }
                            else
                            {
                                painter->setPen(colorEffect);
                            }
                        }
                        else
                        {
                            if (m_trackerview->useInstrumentColorOnEffectAndParameterFrequency() && j % m_trackerview->
                                useInstrumentColorOnEffectAndParameterFrequency() == 0)
                            {
                                painter->setPen(colorInstrumentAlt);
                            }
                            else
                            {
                                painter->setPen(colorEffectAlt);
                            }
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontEffects().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontEffects());
                        fontWidth = m_trackerview->fontWidthEffects();
                    }

                    drawText(effect, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += effect.length() * fontWidth;
                    drawText(m_trackerview->separatorEffect(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorEffect().length() * fontWidth;

                    if (parameter == m_trackerview->emptyParameter() && !m_trackerview->noEmptyParameterColor() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            if (m_trackerview->useInstrumentColorOnEffectAndParameterFrequency() && j % m_trackerview->
                                useInstrumentColorOnEffectAndParameterFrequency() == 0)
                            {
                                painter->setPen(colorInstrument);
                            }
                            else
                            {
                                painter->setPen(colorParameter);
                            }
                        }
                        else
                        {
                            if (m_trackerview->useInstrumentColorOnEffectAndParameterFrequency() && j % m_trackerview->
                                useInstrumentColorOnEffectAndParameterFrequency() == 0)
                            {
                                painter->setPen(colorInstrumentAlt);
                            }
                            else
                            {
                                painter->setPen(colorParameterAlt);
                            }
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontParameters().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontParameters());
                        fontWidth = m_trackerview->fontWidthParameters();
                    }
                    drawText(parameter, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += parameter.length() * fontWidth;
                    drawText(m_trackerview->separatorParameter(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorParameter().length() * fontWidth;

                    if (effect2 == m_trackerview->emptyEffect() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorEffect2);
                        }
                        else
                        {
                            painter->setPen(colorEffect2Alt);
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontEffects().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontEffects());
                        fontWidth = m_trackerview->fontWidthEffects();
                    }
                    drawText(effect2, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += effect2.length() * fontWidth;
                    drawText(m_trackerview->separatorEffect2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorEffect2().length() * fontWidth;

                    if (parameter2 == m_trackerview->emptyParameter() && !m_trackerview->noEmptyParameter2Color() && j
                        != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorParameter2);
                        }
                        else
                        {
                            painter->setPen(colorParameter2Alt);
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontParameters().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontParameters());
                        fontWidth = m_trackerview->fontWidthParameters();
                    }
                    drawText(parameter2, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += parameter2.length() * fontWidth;
                    drawText(m_trackerview->separatorParameter2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorParameter2().length() * fontWidth;
                }
                else
                {
                    if (effect == m_trackerview->emptyEffect() && !m_trackerview->noEmptyEffectColor() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorEffect);
                        }
                        else
                        {
                            painter->setPen(colorEffectAlt);
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontEffects().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontEffects());
                        fontWidth = m_trackerview->fontWidthEffects();
                    }
                    drawText(effect, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += effect.length() * fontWidth;
                    drawText(m_trackerview->separatorEffect(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorEffect().length() * fontWidth;

                    if (effect2 == m_trackerview->emptyEffect() && !m_trackerview->noEmptyEffect2Color() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorEffect2);
                        }
                        else
                        {
                            painter->setPen(colorEffect2Alt);
                        }
                    }
                    drawText(effect2, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += effect2.length() * fontWidth;
                    drawText(m_trackerview->separatorEffect2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorEffect2().length() * fontWidth;

                    if (parameter == m_trackerview->emptyParameter() && !m_trackerview->noEmptyParameterColor() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorParameter);
                        }
                        else
                        {
                            painter->setPen(colorParameterAlt);
                        }
                    }
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontParameters().m_bitmapFontPath)
                    {
                        setFont(m_trackerview->currentRowBitmapFont());
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        setFont(m_trackerview->bitmapFontParameters());
                        fontWidth = m_trackerview->fontWidthParameters();
                    }
                    drawText(parameter, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += parameter.length() * fontWidth;
                    drawText(m_trackerview->separatorParameter(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorParameter().length() * fontWidth;

                    if (parameter2 == m_trackerview->emptyParameter() && !m_trackerview->noEmptyParameterColor() && j !=
                        currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(m_trackerview->colorEmpty());
                        }
                        else
                        {
                            painter->setPen(colorEmptyAlt);
                        }
                    }
                    else
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(colorParameter2);
                        }
                        else
                        {
                            painter->setPen(colorParameter2Alt);
                        }
                    }
                    drawText(parameter2, painter, m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                    numPixels += parameter2.length() * fontWidth;
                    drawText(m_trackerview->separatorParameter2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             height / 2 + yPixelPosition);
                    numPixels += m_trackerview->separatorParameter2().length() * fontWidth;
                }
            }


            if (m_trackerview->rowNumbersLastChannelEnabled())
            {
                if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                    bitmapFontRownumber().m_bitmapFontPath)
                {
                    setFont(m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    setFont(m_trackerview->bitmapFontRownumber());
                    fontWidth = m_trackerview->fontWidthRownumber();
                }
                painter->setPen(colorRowNumber);
                drawText(m_trackerview->separatorRowNumberLast() + row, painter,
                         m_trackerview->xOffsetRow() + numPixels, height / 2 + yPixelPosition);
                numPixels += row.length() * fontWidth;
            }


            //            if(m_trackerview->rowHighlightBackgroundFrequency())
            //            {
            //                if(!(j % m_trackerview->rowHighlightBackgroundFrequency()))
            //                {
            //                    painter->fillRect(m_trackerview->highlightBackgroundOffset(),height/2+yPixelPosition+m_trackerview->yOffsetRowHighlight(),m_trackerview->xOffsetRow()+numPixels-(m_trackerview->highlightBackgroundOffset()),m_trackerview->font().pixelSize(),m_trackerview->colorRowHighlightBackground());
            //                }
            //            }
            //            if(m_trackerview->rowHighlightBackgroundFrequency2())
            //            {
            //                if(!(j % m_trackerview->rowHighlightBackgroundFrequency2()))
            //                {
            //                    painter->fillRect(m_trackerview->highlightBackgroundOffset(),height/2+yPixelPosition+m_trackerview->yOffsetRowHighlight(),m_trackerview->xOffsetRow()+numPixels-(m_trackerview->highlightBackgroundOffset()),m_trackerview->font().pixelSize(),m_trackerview->colorRowHighlightBackground2());
            //                }
            //            }

            if (m_trackerview->linesBetweenRows() && j != currentRow && j != currentRow - 1)
            {
                painter->fillRect(m_trackerview->xOffsetRow(),
                                  height / 2 + yPixelPosition + m_trackerview->yOffsetRowHighlight(),
                                  m_trackerview->xOffsetRow() + numPixels - (m_trackerview->
                                      highlightBackgroundOffset()), 1, QColor(0, 0, 0));
            }
            j++;
            if (m_info->plugin == PLUGIN_libopenmpt || m_info->plugin == PLUGIN_libxmp || m_info->plugin ==
                PLUGIN_sunvox)
            {
                i += m_info->numChannels;
            }
            else if (m_info->plugin == PLUGIN_hivelytracker)
            {
                i++;
            }
        }


        if (m_renderTop)
        {
            setFont(m_trackerview->bitmapFont());
            if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (6chn") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (8chn") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (16ch"))
            {
                m_height = 31;
                QColor colorBase(146, 146, 162);
                QColor colorHilite(255, 255, 255);
                int top = 16;
                int left = 0 + 12;
                QRect rectBg(0, 0, width, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(QColor(255, 255, 255));
                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 88, top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText("POSITION", painter, left + 0, top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText("PATTERN", painter, left + 140, top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (228), top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText("LENGTH", painter, left + 280, top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (368), top + 0);
                painter->setPen(QColor(255, 255, 255));
                drawText("SONGNAME:", painter, left + 0, top + (15));
                painter->setPen(QColor(255, 255, 255));
                drawText(QString(m_info->title.c_str()).toUpper(), painter, left + (80), top + (15));

                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(0, 15, width, 15);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(0, 0, width, 0);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(0, m_height - 1, width - 1, m_height - 1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(1, 0, 1, m_height - 1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(width, 0, width, m_height - 1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + (129), 0, left + (129), 15);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + (269), 0, left + (269), 15);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + (409), 0, left + (409), m_height - 1);
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (patt"))
            {
                m_height = 41;
                QColor colorBase(170, 170, 170);
                QColor colorHilite(255, 255, 255);
                QColor colorShadow(51, 0, 0);
                QColor colorBlue(0, 0, 187);
                int top = 0;
                int left = 0;
                QRect rectBg(left, 0, 544, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(colorShadow);
                setFont(m_trackerview->infoFont());
                drawText("SONGNAME", painter, left + (4), top + (18));
                painter->setPen(colorHilite);
                drawText("SONGNAME", painter, left + (3), top + (16));
                painter->setPen(QColor(colorShadow));
                painter->setPen(colorBlue);
                setFont(m_trackerview->infoFont2());
                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')), painter, left + (65), top + (18));
                painter->setPen(colorShadow);
                setFont(m_trackerview->infoFont());
                drawText("POSITION", painter, left + (234), top + (18));
                painter->setPen(colorHilite);
                drawText("POSITION", painter, left + (233), top + (16));
                painter->setPen(colorShadow);
                painter->setPen(QColor(colorShadow));
                painter->setPen(colorBlue);
                setFont(m_trackerview->infoFont2());
                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (296), top + (18));
                painter->setPen(colorShadow);
                setFont(m_trackerview->infoFont());
                drawText("PATTERN", painter, left + (339), top + (18));
                painter->setPen(colorHilite);
                drawText("PATTERN", painter, left + (338), top + (16));
                painter->setPen(colorBlue);
                setFont(m_trackerview->infoFont2());
                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (401), top + (18));
                painter->setPen(colorShadow);
                setFont(m_trackerview->infoFont());
                drawText("LENGTH", painter, left + (444), top + (18));
                painter->setPen(colorHilite);
                drawText("LENGTH", painter, left + (443), top + (16));
                painter->setPen(QColor(colorShadow));
                painter->setPen(colorBlue);
                setFont(m_trackerview->infoFont2());
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (506), top + (18));
                setFont(m_trackerview->infoFont());
                painter->setPen(colorShadow);
                drawText("POS", painter, left + (13), top + (38));
                painter->setPen(colorHilite);
                drawText("POS", painter, left + (12), top + (36));
                for (int i = 0; i < 4; i++)
                {
                    painter->setPen(QColor(colorShadow));
                    drawText("TRACK #" + QString::number(i + 1), painter, left + (69 + (120 * i)), top + (38), 1);
                    painter->setPen(colorHilite);
                    drawText("TRACK #" + QString::number(i + 1), painter, left + (68 + (120 * i)), top + (36), 1);
                }

                painter->fillRect(left, 0, 1, m_height, colorHilite);
                painter->fillRect(left + 543, 0, 1, m_height, colorShadow);
                painter->fillRect(left, 0, 543, 2, colorHilite);

                painter->fillRect(left, 20, 543, 2, colorShadow);
                painter->fillRect(left, 22, 543, 2, colorHilite);

                //bevels
                painter->fillRect(left + 228, 2, 1, 18, colorShadow);
                painter->fillRect(left + 229, 2, 1, 18, colorHilite);
                painter->fillRect(left + 333, 2, 1, 18, colorShadow);
                painter->fillRect(left + 334, 2, 1, 18, colorHilite);
                painter->fillRect(left + 438, 2, 1, 18, colorShadow);
                painter->fillRect(left + 439, 2, 1, 18, colorHilite);
                //bevels antialiasing
                painter->fillRect(left, 20, 1, 2, colorBase);
                painter->fillRect(left + 228, 0, 1, 2, colorBase);
                painter->fillRect(left + 229, 20, 1, 2, colorBase);
                painter->fillRect(left + 333, 0, 1, 2, colorBase);
                painter->fillRect(left + 334, 20, 1, 2, colorBase);
                painter->fillRect(left + 438, 0, 1, 2, colorBase);
                painter->fillRect(left + 439, 20, 1, 2, colorBase);
                painter->fillRect(left + 543, 0, 1, 2, colorBase);
                painter->fillRect(left + 543, 22, 1, 2, colorBase);
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (flt4)"))
            {
                m_height = 44;
                QColor colorBase(189, 138, 99);
                QColor colorHilite(222, 186, 156);
                QColor colorShadow(140, 85, 49);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 320, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(colorShadow);
                drawText("POSITION", painter, left + (4), top + 1, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 71, top + 0);
                painter->setPen(colorHilite);
                drawText("POSITION", painter, left + (3), top + 0, -1);
                painter->setPen(colorShadow);
                drawText("PATTERN", painter, left + (111), top + 1, -1);
                painter->setPen(colorHilite);
                drawText("PATTERN", painter, left + (110), top + 0, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (178), top + 0);
                painter->setPen(colorShadow);
                drawText("LENGTH", painter, left + (4), top + (11) + 1, -1);
                painter->setPen(colorHilite);
                drawText("LENGTH", painter, left + (3), top + (11), -1);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (71), top + (11));
                painter->setPen(colorShadow);
                drawText("RESTART", painter, left + (111), top + (11) + 1, -1);
                painter->setPen(colorHilite);
                drawText("RESTART", painter, left + (110), top + (11), -1);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_info->restart, 4, 10, QChar('0')), painter, left + (178), top + (11));

                painter->setPen(colorShadow);
                drawText("SONGNAME:", painter, left + 4, top + 23, -1);
                painter->setPen(colorHilite);
                drawText("SONGNAME:", painter, left + (3), top + (22), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (72),
                         top + (22));

                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 0, left + (319), 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 10, left + (215), 10);
                painter->drawLine(left + 1, 21, left + (319), 21);
                painter->drawLine(left + 1, 32, left + (319), 32);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, (11), left + (215), (11));
                painter->drawLine(left + 1, (22), left + (319), (22));
                painter->drawLine(left + 1, (33), left + (319), (33));

                drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + 68, 11, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 11, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 11, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 11, 10, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + 214, 0, 21, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left + 10, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);
                drawVerticalEmboss(left + 21, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);
                drawVerticalEmboss(left + 44, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);

                //far left emboss
                drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 22, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 33, 10, colorHilite, colorShadow, colorBase, painter, false, true);

                //far right emboss
                drawVerticalEmboss(left + (319), 0, 22, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 22, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 33, 11, colorHilite, colorShadow, colorBase, painter, true, false);


                QRectF sourcePrev(0, 0, 6, 7);
                QRectF sourceNext(6, 0, 6, 7);
                QRectF targetPrev(left + 3, 35, 6, 7);
                QRectF targetNext(left + 14, 35, 6, 7);
                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "startrekker_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPrev, imageTop, sourcePrev);
                painter->drawImage(targetNext, imageTop, sourceNext);

                painter->setPen(colorShadow);
                drawText("SAMPLENAME:", painter, left + (59), top + (33) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SAMPLENAME:", painter, left + (58), top + (33), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 2, 10, QChar('0')), painter,
                         left + (24), top + (33));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + (141), top + (33));

                //1px antialias
                painter->fillRect(left, 32, 1, 1, colorBase);
                painter->fillRect(left + 319, 33, 1, 1, colorBase);

                //bottom shadow
                painter->fillRect(left, 43, 320, 1, colorShadow);
            }
            else if ((QString(m_info->fileformat.c_str()).toLower().startsWith("protracker") && !
                    QString(m_info->fileformat.c_str()).toLower().startsWith("protracker xm")) ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("probably converted (m.k") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("unknown/converted (m.k") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("converted 15 ins") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("unknown or converted (M"))
            {
                m_height = 32;
                QColor colorBase(136, 136, 136);
                QColor colorHilite(187, 187, 187);
                QColor colorShadow(85, 85, 85);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 319, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(colorShadow);
                drawText("POSITION", painter, left + (4), top + 1, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 71, top + 0);

                painter->setPen(colorHilite);
                drawText("POSITION", painter, left + (3), top, -1);
                painter->setPen(colorShadow);
                drawText("PATTERN", painter, left + (111), top + 1, -1);
                painter->setPen(colorHilite);
                drawText("PATTERN", painter, left + (110), top + 0, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (178), top + 0);

                painter->setPen(colorShadow);
                drawText("LENGTH", painter, left + (218), top + 1, -1);
                painter->setPen(colorHilite);
                drawText("LENGTH", painter, left + (217), top + 0, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (285), top + 0);

                painter->setPen(colorShadow);
                drawText("SONGNAME:", painter, left + (73), top + (11) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SONGNAME:", painter, left + (72), top + (11), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (141),
                         top + (11));

                QRectF sourcePrev(0, 0, 6, 7);
                QRectF sourceNext(6, 0, 6, 7);
                QRectF targetPrev(left + 3, 24, 6, 7);
                QRectF targetNext(left + 14, 24, 6, 7);
                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "protracker_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPrev, imageTop, sourcePrev);
                painter->drawImage(targetNext, imageTop, sourceNext);

                painter->setPen(colorShadow);
                drawText("SAMPLENAME:", painter, left + (59), top + (22) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SAMPLENAME:", painter, left + (58), top + (22), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 2, 10, QChar('0')), painter,
                         left + (24), top + (22));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + 141, top + 22);
                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 0, left + (319), 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left, 10, left + 319, 10);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 11, left + 319, 11);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left, 21, left + 319, 21);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 22, left + 319, 22);


                drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 44, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 281, 0, 10, colorHilite, colorShadow, colorBase, painter);

                //far left emboss
                drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                //drawVerticalEmboss(left-1,22,16,colorHilite,colorShadow,colorBase,painter,false,true);
                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 22, left + 1, 33);

                //far right emboss
                drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 11, 10, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 22, 10, colorHilite, colorShadow, colorBase, painter, true, false);
            }


            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("chiptracker"))
            {
                m_height = 32;
                QColor colorBase(115, 117, 115);
                QColor colorHilite(173, 170, 173);
                QColor colorShadow(66, 69, 66);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 320, m_height);
                painter->fillRect(rectBg, colorBase);
                QRectF sourcePosition(0, 0, 60, 6);
                QRectF targetPosition(left + 4, 3, 60, 6);
                QRectF sourcePattern(0, 6, 55, 6);
                QRectF targetPattern(left + 111, 3, 55, 6);
                QRectF sourceLength(0, 12, 48, 6);
                QRectF targetLength(left + 218, 3, 48, 6);
                QRectF sourceSongname(0, 18, 70, 6);
                QRectF targetSongname(left + 62, 14, 70, 6);
                QRectF sourcePrev(58, 6, 6, 7);
                QRectF sourceNext(64, 6, 6, 7);
                QRectF targetPrev(left + 3, 24, 6, 7);
                QRectF targetNext(left + 14, 24, 6, 7);

                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "chiptracker_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPosition, imageTop, sourcePosition);
                painter->drawImage(targetPattern, imageTop, sourcePattern);
                painter->drawImage(targetLength, imageTop, sourceLength);
                painter->drawImage(targetSongname, imageTop, sourceSongname);
                painter->drawImage(targetPrev, imageTop, sourcePrev);
                painter->drawImage(targetNext, imageTop, sourceNext);

                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPosition, 4, 16, QChar('0')).toUpper(), painter, left + 71,
                         top + 0);
                drawText(QString("%1").arg(m_info->restart, 4, 16, QChar('0')).toUpper(), painter, left + (178),
                         top + 0);
                drawText(QString("%1").arg(m_info->numOrders, 4, 16, QChar('0')).toUpper(), painter, left + (285),
                         top + 0);
                drawText(QString("%1").arg(m_info->title.c_str(), -22, QChar('_')).toUpper(), painter, left + (137),
                         top + (11));

                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 0, 319, 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 10, 319, 10);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 11, 319, 11);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 21, 319, 21);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 22, 319, 22);

                drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 281, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 58, 22, 10, colorHilite, colorShadow, colorBase, painter);

                //far left emboss
                drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 11, left + 1, 33);

                //far right emboss
                drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 11, 21, colorHilite, colorShadow, colorBase, painter, true, false);

                //1px antialias
                painter->fillRect(left + (319), 22, 1, 1, colorBase);
                painter->fillRect(left, 21, 1, 1, colorBase);

                painter->setPen(colorShadow);
                drawText("SAMPNAME:", painter, left + (63), top + 23);
                painter->setPen(colorHilite);
                drawText("SAMPNAME:", painter, left + (62), top + 22);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 4, 16, QChar('0')).toUpper(), painter,
                         left + (24), top + (22));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + (137), top + (22));
            }

            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("ice tracker"))
            {
                m_height = 32;
                QColor colorBase(115, 138, 156);
                QColor colorHilite(173, 186, 206);
                QColor colorShadow(66, 85, 115);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 319, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(colorShadow);
                drawText("POSITION", painter, left + (4), top + 1, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (71), top + 0);
                painter->setPen(colorHilite);
                drawText("POSITION", painter, left + (3), top + 0, -1);
                painter->setPen(colorShadow);
                drawText("LENGTH", painter, left + (111), top + 1, -1);
                painter->setPen(colorHilite);
                drawText("LENGTH", painter, left + (110), top + 0, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (178), top + 0);
                painter->setPen(colorShadow);
                drawText("SONGNAME:", painter, left + (73), top + (11) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SONGNAME:", painter, left + (72), top + (11), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (141),
                         top + (11));
                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 0, left + (319), 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left, 10, left + (319), 10);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 11, left + (319), 11);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left, 21, left + (319), 21);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 22, left + (319), 22);

                drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 44, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);

                //far left emboss
                drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 22, 10, colorHilite, colorShadow, colorBase, painter, false, true);

                //far right emboss
                drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 11, 10, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 22, 10, colorHilite, colorShadow, colorBase, painter, true, false);

                painter->setPen(colorShadow);
                drawText("SAMPLENAME:", painter, left + (59), top + (22) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SAMPLENAME:", painter, left + (58), top + (22), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 2, 10, QChar('0')), painter,
                         left + (24), top + (22));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + (141), top + (22));

                QRectF sourcePrev(0, 0, 6, 7);
                QRectF sourceNext(6, 0, 6, 7);
                QRectF targetPrev(left + 3, 24, 6, 7);
                QRectF targetNext(left + 14, 24, 6, 7);
                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "icetracker_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPrev, imageTop, sourcePrev);
                painter->drawImage(targetNext, imageTop, sourceNext);
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("multitracker"))
            {
                m_height = 25;
                QColor colorBase(0, 0, 85);
                int numChannels = m_info->numChannels;
                int top = 16;
                int left = 0;
                QRect rectBg(left, 0, (64 + (numChannels * 72)), m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(QColor(255, 255, 255));
                for (int i = 0; i < numChannels; i++)
                {
                    drawText("Track:" + QString::number(i + 1), painter, left + ((40 + i * 72)), top);
                }
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("mnemotron"))
            {
                m_height = 32;
                QColor colorBase(115, 117, 115);
                QColor colorHilite(173, 170, 173);
                QColor colorShadow(66, 69, 66);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 320, m_height);
                painter->fillRect(rectBg, colorBase);

                painter->setPen(colorShadow);
                drawText("POSITION", painter, left + (4), top + 1, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (71), top + 0);
                painter->setPen(colorHilite);
                drawText("POSITION", painter, left + (3), top + 0, -1);
                painter->setPen(colorShadow);
                drawText("LENGTH", painter, left + (111), top + 1, -1);
                painter->setPen(colorHilite);
                drawText("LENGTH", painter, left + (110), top + 0, -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (178), top + 0);
                painter->setPen(colorShadow);
                drawText("SONGNAME:", painter, left + (73), top + (11) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SONGNAME:", painter, left + (72), top + (11), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (141),
                         top + (11));
                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 0, left + (319), 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 10, left + (319), 10);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 11, left + (319), 11);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 21, left + (319), 21);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 22, left + (319), 22);

                drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 44, 22, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
                drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);

                //far left emboss
                drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
                drawVerticalEmboss(left - 1, 22, 10, colorHilite, colorShadow, colorBase, painter, false, true);

                //far right emboss
                drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 11, 11, colorHilite, colorShadow, colorBase, painter, true, false);
                drawVerticalEmboss(left + (319), 22, 11, colorHilite, colorShadow, colorBase, painter, true, false);

                painter->setPen(colorShadow);
                drawText("SAMPLENAME:", painter, left + (59), top + (22) + 1, -1);
                painter->setPen(colorHilite);
                drawText("SAMPLENAME:", painter, left + (58), top + (22), -1);
                painter->setPen(QColor(0, 0, 0));

                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 2, 10, QChar('0')), painter,
                         left + (24), top + (22));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + (141), top + (22));
                QRectF sourcePrev(0, 0, 6, 7);
                QRectF sourceNext(6, 0, 6, 7);
                QRectF targetPrev(left + 3, 24, 6, 7);
                QRectF targetNext(left + 14, 24, 6, 7);
                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "noisetracker_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPrev, imageTop, sourcePrev);
                painter->drawImage(targetNext, imageTop, sourceNext);
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("fasttracker 2") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("skale tracker xm") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("madtracker 2.0 xm") || QString(
                    m_info->fileformat.c_str()).toLower().endsWith("xm 1.04"))
            {
                m_height = 29;
                QColor colorBase(73, 117, 130);
                QColor colorHilite(138, 219, 243);
                QColor colorShadow(24, 40, 44);
                QColor colorWhite(255, 255, 255);
                int left = 0;

                //background
                if (m_info->numChannels > 4 && m_info->numChannels < 7)
                {
                    painter->fillRect(left, 0, 55 + (m_info->numChannels * 96 * m_trackerview->fontWidth() / 8),
                                      m_height, colorBase);
                }
                else
                {
                    painter->fillRect(left, 0, 55 + (m_info->numChannels * 72 * m_trackerview->fontWidth() / 8),
                                      m_height, colorBase);
                }

                //left
                painter->fillRect(left, 0, 1, m_height - 1, colorHilite);

                //right
                if (m_info->numChannels > 4 && m_info->numChannels < 7)
                {
                    painter->fillRect(left + 55 + (m_info->numChannels * 96 * m_trackerview->fontWidth() / 8), 0, 1,
                                      m_height, colorShadow);
                }
                else
                {
                    painter->fillRect(left + 55 + (m_info->numChannels * 72 * m_trackerview->fontWidth() / 8), 0, 1,
                                      m_height, colorShadow);
                }

                //top
                if (m_info->numChannels > 4 && m_info->numChannels < 7)
                {
                    painter->fillRect(left, 0, 55 + (m_info->numChannels * 96 * m_trackerview->fontWidth() / 8), 1,
                                      colorHilite);
                }
                else
                {
                    painter->fillRect(left, 0, 55 + (m_info->numChannels * 72 * m_trackerview->fontWidth() / 8), 1,
                                      colorHilite);
                }

                //bottom
                if (m_info->numChannels > 4 && m_info->numChannels < 7)
                {
                    painter->fillRect(left, m_height - 1,
                                      55 + (m_info->numChannels * 96 * m_trackerview->fontWidth() / 8), 1, colorShadow);
                }
                else
                {
                    painter->fillRect(left, m_height - 1,
                                      55 + (m_info->numChannels * 72 * m_trackerview->fontWidth() / 8), 1, colorShadow);
                }

                setFont(m_trackerview->infoFont2());
                painter->setPen(colorShadow);
                drawText("Songlen.", painter, left + 4, 15);
                painter->setPen(colorWhite);
                drawText("Songlen.", painter, left + 3, 14);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_info->numOrders, 2, 16, QChar('0')).toUpper(), painter, left + 58, 14);
                setFont(m_trackerview->infoFont2());

                painter->setPen(colorShadow);
                drawText("Repstart", painter, left + 4, 27);
                painter->setPen(colorWhite);
                drawText("Repstart", painter, left + 3, 26);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_info->restart, 2, 16, QChar('0')).toUpper(), painter, left + 58, 26);
                //bevel
                painter->fillRect(left + (75), 1, 1, (m_height - 2), colorShadow);
                painter->fillRect(left + (76), 1, 1, (m_height - 2), colorHilite);

                setFont(m_trackerview->infoFont2());

                painter->setPen(colorShadow);
                drawText("BPM", painter, left + 80, 15);
                painter->setPen(colorWhite);
                drawText("BPM", painter, left + 79, 14);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_currentBPM, 3, 10, QChar('0')), painter, left + 108, 14);
                setFont(m_trackerview->infoFont2());

                painter->setPen(colorShadow);
                drawText("Spd.", painter, left + 80, 27);
                painter->setPen(colorWhite);
                drawText("Spd.", painter, left + 79, 26);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_currentSpeed, 2, 16, QChar('0')).toUpper(), painter, left + 115, 26);
                //bevel
                painter->fillRect(left + (132), 1, 1, (m_height - 2), colorShadow);
                painter->fillRect(left + (133), 1, 1, (m_height - 2), colorHilite);


                setFont(m_trackerview->infoFont2());

                painter->setPen(colorShadow);
                drawText("Ptn.", painter, left + 137, 15);
                painter->setPen(colorWhite);
                drawText("Ptn.", painter, left + 136, 14);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_currentPattern, 2, 16, QChar('0')).toUpper(), painter, left + 163, 14);
                setFont(m_trackerview->infoFont2());

                painter->setPen(colorShadow);
                drawText("Ln.", painter, left + 137, 27);
                painter->setPen(colorWhite);
                drawText("Ln.", painter, left + 136, 26);
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_info->modPatternRows, 3, 16, QChar('0')).toUpper(), painter, left + 156,
                         26);
                //title frame

                int frameOffsetX = 181;
                int frameOffsetY = 11;
                painter->fillRect(left + frameOffsetX, frameOffsetY, 165, 1, colorHilite);
                painter->fillRect(left + frameOffsetX, 1, 1, 27, colorHilite);
                painter->fillRect(left + (frameOffsetX + 3), (frameOffsetY + 15), 161, 1, colorHilite);
                painter->fillRect(left + (frameOffsetX + 163), (frameOffsetY + 3), 1, 12, colorHilite);
                painter->fillRect(left + (frameOffsetX + 2), (frameOffsetY + 2), 162, 1, colorShadow);
                painter->fillRect(left + (frameOffsetX + 2), (frameOffsetY + 3), 1, 13, colorShadow);
                painter->fillRect(left + (frameOffsetX + 3), (frameOffsetY + 3), 160, 12, QColor(0, 0, 0));

                painter->fillRect(left + (frameOffsetX - 1), 1, 1, (m_height - 2), colorShadow);
                painter->fillRect(left + (frameOffsetX + 165), 1, 1, (m_height - 2), colorShadow);
                painter->fillRect(left + (frameOffsetX + 166), 1, 1, (m_height - 2), colorHilite);

                painter->setPen(colorWhite);

                setFont(m_trackerview->infoFont2());
                drawText(QString(m_info->title.c_str()), painter, left + (frameOffsetX + 4), (frameOffsetY + 14));
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0"))
            {
                m_height = 32;
                QColor colorBase(153, 153, 170);
                QColor colorHilite(204, 204, 204);
                QColor colorShadow(102, 102, 119);
                int left = 0;

                //left
                painter->fillRect(left, 0, 4, 32, colorShadow);
                painter->fillRect(left + 4, 4, 2, 28, colorBase);

                //right
                painter->fillRect(left + 636, 4, 4, 28, colorHilite);
                painter->fillRect(left + 634, 30, 2, 2, colorBase);

                //top
                painter->fillRect(left + 4, 4, 632, 26, colorBase);
                painter->fillRect(left + 1, 0, 639, 2, colorHilite);
                painter->fillRect(left + 3, 2, 637, 2, colorHilite);

                //bottom
                painter->fillRect(left + 6, 30, 1, 2, colorHilite);
                painter->fillRect(left + 7, 30, 627, 2, colorShadow);

                //position info etc.
                painter->fillRect(left + 6, 6, 154, 22, colorShadow);
                painter->fillRect(left + 7, 8, 83, 2, colorHilite);
                painter->fillRect(left + 91, 8, 67, 2, colorHilite);
                painter->fillRect(left + 88, 10, 2, 16, colorHilite);
                painter->fillRect(left + 156, 10, 2, 16, colorHilite);
                painter->fillRect(left + 89, 26, 1, 2, colorHilite);
                painter->fillRect(left + 157, 26, 1, 2, colorHilite);

                painter->fillRect(left + 8, 10, 80, 16, colorBase);
                painter->fillRect(left + 92, 10, 64, 16, colorBase);

                setFont(m_trackerview->infoFont());

                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_currentPosition + 1, 4, 10, QChar('0')), painter, left + 10, 24);
                drawText("/", painter, left + 44, 24);
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + 54, 24);
                drawText(QString("%1").arg(m_currentPattern, 3, 10, QChar('0')), painter, left + 94, 24);
                drawText("/", painter, left + 120, 24);
                drawText(QString("%1").arg(m_info->numPatterns, 3, 10, QChar('0')), painter, left + 130, 24);
            }

            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1"))
            {
                m_height = 28;
                QColor colorBase(153, 153, 170);
                QColor colorHilite(204, 204, 204);
                QColor colorShadow(102, 102, 119);
                int left = 0;

                //background
                painter->fillRect(left + 2, 0, 636, m_height, colorBase);

                //left
                painter->fillRect(left, 0, 2, m_height, colorShadow);


                //right
                painter->fillRect(left + 638, 2, 2, m_height, colorHilite);

                //top
                painter->fillRect(left + 1, 0, 639, 2, colorHilite);


                //bottom
                painter->fillRect(left + 1, 26, 638, 2, colorShadow);

                //position info etc.
                painter->fillRect(left + 3, 4, 253, 22, colorShadow);
                painter->fillRect(left + 4, 4, 83, 2, colorHilite);
                painter->fillRect(left + 88, 4, 67, 2, colorHilite);
                painter->fillRect(left + 156, 4, 35, 2, colorHilite);
                painter->fillRect(left + 192, 4, 64, 2, colorHilite);
                painter->fillRect(left + 85, 6, 2, 18, colorHilite);
                painter->fillRect(left + 153, 6, 2, 18, colorHilite);
                painter->fillRect(left + 189, 6, 2, 18, colorHilite);
                painter->fillRect(left + 254, 6, 2, 18, colorHilite);
                painter->fillRect(left + 86, 24, 1, 2, colorHilite);
                painter->fillRect(left + 154, 24, 1, 2, colorHilite);
                painter->fillRect(left + 190, 24, 1, 2, colorHilite);
                painter->fillRect(left + 255, 24, 1, 2, colorHilite);

                painter->fillRect(left + 5, 6, 80, 18, colorBase);
                painter->fillRect(left + 89, 6, 64, 18, colorBase);
                painter->fillRect(left + 157, 6, 32, 18, colorBase);
                painter->fillRect(left + 193, 6, 61, 18, colorBase);

                painter->setPen(QColor(0, 0, 0));
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_currentPosition + 1, 4, 10, QChar('0')), painter, left + 6, 24);
                drawText("/", painter, left + 40, 24);
                drawText("/", painter, left + 41, 24);
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + 50, 24);
                drawText(QString("%1").arg(m_currentPattern, 3, 10, QChar('0')), painter, left + 90, 24);
                drawText("/", painter, left + 116, 24);
                drawText("/", painter, left + 117, 24);
                drawText(QString("%1").arg(m_info->numPatterns, 3, 10, QChar('0')), painter, left + 126, 24);
                drawText("SPD", painter, left + 161, 24);
                drawText(QString("%1").arg(m_currentBPM, 3, 10, QChar('0')), painter, left + 196, 24);
                drawText("/", painter, left + 223, 24);
                drawText("/", painter, left + 224, 24);
                drawText(QString("%1").arg(m_currentSpeed, 2, 16, QChar('0')), painter, left + 235, 24);
            }

            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd3"))
            {
                m_height = 32;
                QColor colorBase(156, 154, 156);
                QColor colorHilite(255, 255, 255);
                QColor colorShadow(0, 0, 0);
                QColor colorRed(255, 0, 0);
                int left = 0;
                int minChannels = m_info->numChannels < 11 ? 11 : m_info->numChannels;
                //top, songname
                //background
                painter->fillRect(left, 0, (25 + (minChannels * 60)), 15, colorHilite);
                painter->fillRect(left, 15, (25 + (minChannels * 60)), 1, colorShadow);
                //right
                painter->fillRect(left + (24 + (minChannels * 60)), 0, 1, 16, colorShadow);
                //bottom
                painter->fillRect(left, 16, (25 + (minChannels * 60)), 1, colorShadow);


                //block
                //background
                painter->fillRect(left, 16, (25 + (minChannels * 60)), 16, colorBase);
                //left
                painter->fillRect(left, 16, 1, 16, colorHilite);
                //right
                painter->fillRect(left + (24 + (minChannels * 60)), 16, 1, height, colorShadow);
                //top
                painter->fillRect(left, 16, (25 + (minChannels * 60)), 1, colorHilite);
                //bottom
                painter->fillRect(left, m_height - (1), (25 + (minChannels * 60)), 1, colorShadow);

                setFont(m_trackerview->infoFont());
                painter->setPen(QColor(0, 0, 0));
                drawText(
                    QString("Block ") + QString::number(m_currentPattern) + "/" + QString::number(
                        m_info->numPatterns - 1), painter, left + 30, 30);
                drawText("Song: " + QString(m_info->title.c_str()), painter, left + 6, 14);
            }

            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
            {
                m_height = 54;
                QColor colorBase(156, 154, 156);
                QColor colorHilite(255, 255, 255);
                QColor colorShadow(0, 0, 0);

                setFont(m_trackerview->infoFont());

                int left = 0;

                //top, songname


                //background
                painter->fillRect(left, 0, 640, m_height, colorBase);
                painter->fillRect(left, 0, 640, 20, colorHilite);
                painter->fillRect(left, 20, 640, 4, colorShadow);
                painter->fillRect(left, 50, 640, 4, colorShadow);

                //right
                painter->fillRect(left + 639, 2, 1, 18, colorShadow);

                //buttons
                painter->fillRect(left, 22, 2, 26, colorHilite);
                painter->fillRect(left + 71, 22, 4, 26, colorHilite);
                painter->fillRect(left + 160, 22, 4, 26, colorHilite);
                painter->fillRect(left + 249, 22, 4, 26, colorHilite);
                painter->fillRect(left + 634, 22, 2, 26, colorHilite);
                painter->fillRect(left + 71, 22, 1, 2, colorShadow);
                painter->fillRect(left + 160, 22, 1, 2, colorShadow);
                painter->fillRect(left + 249, 22, 1, 2, colorShadow);
                painter->fillRect(left + 634, 22, 1, 2, colorShadow);
                painter->fillRect(left + 22, 22, 4, 26, colorShadow);
                painter->fillRect(left + 95, 22, 4, 26, colorShadow);
                painter->fillRect(left + 184, 22, 4, 26, colorShadow);
                painter->fillRect(left + 265, 22, 4, 26, colorShadow);
                painter->fillRect(left + 636, 22, 4, 2, colorBase);
                painter->fillRect(left, 22, 23, 2, colorHilite);
                painter->fillRect(left + 72, 22, 24, 2, colorHilite);
                painter->fillRect(left + 161, 22, 24, 2, colorHilite);
                painter->fillRect(left + 250, 22, 16, 2, colorHilite);
                painter->fillRect(left + 1, 46, 24, 2, colorShadow);
                painter->fillRect(left + 1, 46, 24, 2, colorShadow);
                painter->fillRect(left + 74, 46, 24, 2, colorShadow);
                painter->fillRect(left + 163, 46, 24, 2, colorShadow);
                painter->fillRect(left + 252, 46, 16, 2, colorShadow);
                painter->fillRect(left + 252, 46, 16, 2, colorShadow);

                painter->fillRect(left + 25, 46, 49, 2, colorHilite);
                painter->fillRect(left + 25, 46, 49, 2, colorHilite);
                painter->fillRect(left + 98, 46, 65, 2, colorHilite);
                painter->fillRect(left + 187, 46, 65, 2, colorHilite);
                painter->fillRect(left + 268, 46, 368, 2, colorHilite);
                painter->fillRect(left, 50, 639, 2, colorHilite);
                painter->fillRect(left, 52, 1, 2, colorHilite);

                painter->setPen(QColor(0, 0, 0));
                drawText("Sg", painter, left + (4), 44);
                drawText("Sc", painter, left + (77), 44);
                drawText("Sq", painter, left + (166), 44);
                drawText(
                    QString("%1").arg(m_currentPosition + 1, 3, 10, QChar('0')) + "/" + QString("%1").arg(
                        m_info->numOrders, 3, 10, QChar('0')), painter, left + (191), 44);
                drawText("B", painter, left + (255), 44);
                drawText(
                    QString("%1").arg(m_currentPattern, 3, 10, QChar('0')) + "/" + QString("%1").arg(
                        m_info->numPatterns - 1, 3, 10, QChar('0')) + ":", painter, left + (272), 44);

                //underline
                painter->fillRect(left + (12), 42, 1, 2, colorShadow);
                painter->fillRect(left + (19), 42, 1, 2, colorShadow);
                painter->fillRect(left + (85), 42, 8, 2, colorShadow);
                painter->fillRect(left + (174), 42, 4, 2, colorShadow);
                painter->fillRect(left + (255), 42, 8, 2, colorShadow);

                drawText("Song: " + QString::fromLocal8Bit(m_info->title.c_str()), painter, left + (5), 18);
            }

            else if (QString(m_info->fileformat.c_str()) == "AHX")
            {
                //TODO
                //some characters missing in font, copyright symbol, maybe others

                m_height = 42;
                int top = 0;
                int left = 0;
                QColor colorBase(140, 134, 146);
                QColor colorHilite(255, 255, 255);
                QColor colorShadow(82, 89, 99);


                int imageWidth = 320;
                int imageHeight = 42;

                QRectF source(0, 0, imageWidth, imageHeight);
                QRectF target(left, top, imageWidth, imageHeight);

                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "ahx_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(target, imageTop, source);

                setFont(m_trackerview->infoFont());

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);

                for (int i = 0; i < m_info->modTrackPositions.size(); i++)
                {
                    drawText(QString("%1").arg(m_info->modTrackPositions[i], 3, 10, QChar('0')), painter,
                             left + (32) + (i * 72), top + (40));
                }


                drawText(QString("%1").arg(m_currentPosition, 3, 10, QChar('0')), painter, left + 26, top + (11));
                drawText(QString("%1").arg(m_info->numOrders, 3, 10, QChar('0')), painter, left + 76, top + (11));
                drawText(QString("%1").arg(m_info->modPatternRestart, 3, 10, QChar('0')), painter, left + 126,
                         top + (11));
                drawText(QString("%1").arg(m_info->modPatternRows, 3, 10, QChar('0')), painter, left + 176, top + (11));
                drawText(QString("%1").arg(m_info->numSubsongs, 3, 10, QChar('0')), painter, left + 219, top + (11));
                drawText(QString::fromLocal8Bit(m_info->title.c_str()).left(35), painter, left + (33), top + (24));
            }
            else if (QString(m_info->fileformat.c_str()) == "HivelyTracker")
            {
                m_height = 104;
                int top = 2;
                int left = 0;
                QColor colorBase(170, 204, 238);
                QColor colorShadow(17, 51, 85);


                QRect rectBg(0, 0, width, m_height);
                painter->fillRect(rectBg, QColor(0, 0, 0));


                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "hively_top.png";
                QImage imageTop(imagepath);


                QFont font = m_trackerview->font();

                QRectF sourceTrack(0, 0, 120, 25);

                painter->setFont(font);
                for (int i = 0; i < m_info->modTrackPositions.size(); i++)
                {
                    QRectF targetTrack(left + (15) + (i * 120), m_height - 25, 120, 25);
                    painter->drawImage(targetTrack, imageTop, sourceTrack);
                    m_pen.setColor(colorBase);
                    painter->setPen(m_pen);
                    font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
                    painter->setFont(font);
                    painter->drawText(left + (108) + (i * 120), m_height - 8,
                                      QString("%1").arg(m_info->modTrackPositions[i], 3, 10, QChar('0')));
                    m_pen.setColor(colorShadow);
                    painter->setPen(m_pen);
                    font.setLetterSpacing(QFont::AbsoluteSpacing, -1);
                    painter->setFont(font);
                    painter->drawText(left + (25) + (i * 120), m_height - 7, "Track " + QString::number(i + 1));
                    m_pen.setColor(QColor(255, 255, 255));
                    painter->setPen(m_pen);
                    painter->drawText(left + (24) + (i * 120), m_height - 8, "Track " + QString::number(i + 1));
                }
                font.setLetterSpacing(QFont::AbsoluteSpacing, 0);

                QFont fontsmall = m_trackerview->font();
                fontsmall.setStyleStrategy(QFont::NoAntialias);

                //song square
                QRectF sourceSongs(166, 0, 337, 78);
                QRectF targetSongs(left + 265, 0, 337, 78);
                painter->drawImage(targetSongs, imageTop, sourceSongs);

                //pattern, position etc. square
                QRectF sourceInfo(503, 0, 263, 78);
                QRectF targetInfo(left, 0, 263, 78);
                painter->drawImage(targetInfo, imageTop, sourceInfo);


                fontsmall.setPixelSize(13);
                painter->setFont(fontsmall);
                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawText(left + 8, top + (24), "Position");
                m_pen.setColor(QColor(255, 255, 255));
                painter->setPen(m_pen);
                painter->drawText(left + 7, top + (23), "Position");

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawText(left + 8, top + (44), "Length");
                m_pen.setColor(QColor(255, 255, 255));
                painter->setPen(m_pen);
                painter->drawText(left + 7, top + (43), "Length");

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawText(left + 8, top + (64), "Restart");
                m_pen.setColor(QColor(255, 255, 255));
                painter->setPen(m_pen);
                painter->drawText(left + 7, top + (63), "Restart");

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawText(left + 130, top + (24), "Track Len");
                m_pen.setColor(QColor(255, 255, 255));
                painter->setPen(m_pen);
                painter->drawText(left + 129, top + (23), "Track Len");

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawText(left + 130, top + (44), "Subsongs");
                m_pen.setColor(QColor(255, 255, 255));
                painter->setPen(m_pen);
                painter->drawText(left + 129, top + (43), "Subsongs");


                painter->setFont(font);
                m_pen.setColor(colorBase);
                painter->setPen(m_pen);
                painter->drawText(left + (96), top + (24), QString("%1").arg(m_currentPosition, 3, 10, QChar('0')));
                painter->drawText(left + (96), top + (44), QString("%1").arg(m_info->numOrders, 3, 10, QChar('0')));
                painter->drawText(left + (96), top + (64),
                                  QString("%1").arg(m_info->modPatternRestart, 3, 10, QChar('0')));
                painter->drawText(left + (217), top + (24),
                                  QString("%1").arg(m_info->modPatternRows, 3, 10, QChar('0')));
                painter->drawText(left + (217), top + (44), QString("%1").arg(m_info->numSubsongs, 3, 10, QChar('0')));
                painter->drawText(left + (275), top + (43), QString(m_info->title.c_str()));

                //far left frame
                QRectF sourceLeftFrameSource(0, 25, 25, 25);
                QRectF targetLeftFrameTarget(left, 79, 25, 25);
                painter->drawImage(targetLeftFrameTarget, imageTop, sourceLeftFrameSource);

                //right frame
                QRectF rightFrameSource(18, 25, 13, 25);
                QRectF rightFrameTarget(left + m_info->modTrackPositions.size() * 120 + (15), 79, 13, 25);
                painter->drawImage(rightFrameTarget, imageTop, rightFrameSource);
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("digibooster"))
            {
                m_height = 46;
                QColor fontColor(222, 186, 140);

                int top = 18;
                int left = 0 + 4;
                int imageWidth = 640;
                int imageHeight = 46;

                QRectF source(0, 0, imageWidth, imageHeight);
                QRectF target(left - 4, 0, imageWidth, imageHeight);

                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "digibooster17_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(target, imageTop, source);

                painter->setPen(QColor(0, 0, 0));
                setFont(m_trackerview->infoFont());
                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (122), top + (2));
                painter->setPen(fontColor);
                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (122), top);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (331), top + (2));
                painter->setPen(fontColor);
                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (331), top);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (540), top + (2));
                painter->setPen(fontColor);
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (540), top);
                painter->setPen(QColor(0, 0, 0));
                drawText(QString(m_info->title.c_str()), painter, left + (72), top + (24));
                painter->setPen(fontColor);
                drawText(QString(m_info->title.c_str()), painter, left + (72), top + (22));
            }

            else if (QString(m_info->fileformat.c_str()).toLower() == "soundtracker")
            {
                m_height = 32;
                QColor colorBase(156, 117, 82);
                QColor colorHilite(173, 138, 99);
                QColor colorShadow(82, 48, 16);
                int top = 8;
                int left = 0;
                QRect rectBg(left, 0, 320, m_height);
                painter->fillRect(rectBg, colorBase);


                QRectF sourcePosition(0, 0, 47, 5);
                QRectF targetPosition(left + 5, 3, 47, 5);
                QRectF sourcePattern(0, 5, 48, 5);
                QRectF targetPattern(left + 111, 3, 48, 5);
                QRectF sourceLength(0, 10, 40, 5);
                QRectF targetLength(left + 218, 3, 40, 5);
                QRectF sourceSongname(0, 15, 61, 5);
                QRectF targetSongname(left + 50, 14, 61, 5);
                QRectF sourceSamplename(0, 20, 75, 5);
                QRectF targetSamplename(left + 50, 25, 75, 5);
                QRectF sourceNextSample(61, 0, 9, 7);
                QRectF targetNextSample(left + 2, 24, 9, 7);
                QRectF sourcePrevSample(70, 0, 9, 7);
                QRectF targetPrevSample(left + 14, 24, 9, 7);
                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "ust_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(targetPosition, imageTop, sourcePosition);
                painter->drawImage(targetPattern, imageTop, sourcePattern);
                painter->drawImage(targetLength, imageTop, sourceLength);
                painter->drawImage(targetSongname, imageTop, sourceSongname);
                painter->drawImage(targetSamplename, imageTop, sourceSamplename);
                painter->drawImage(targetPrevSample, imageTop, sourcePrevSample);
                painter->drawImage(targetNextSample, imageTop, sourceNextSample);


                painter->setPen(QColor(0, 0, 0));
                drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (71), top + 0);
                drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (178), top + 0);
                drawText(QString("%1").arg(m_info->numOrders, 4, 10, QChar('0')), painter, left + (285), top + 0);
                drawText(QString("%1").arg(m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (143),
                         top + (11));
                drawText(QString("%1").arg(m_trackerview->getCurrentSample() + 1, 2, 16, QChar('0')).toUpper(), painter,
                         left + (26), top + (22));
                drawText(
                    QString("%1").arg(m_info->samples[m_trackerview->getCurrentSample()].c_str(), -22, QChar('_')).
                                  toUpper(), painter, left + (143), top + (22));
                m_pen.setWidth(1);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 0, left + 319, 0);

                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left, 10, left + (319), 10);
                painter->drawLine(left, 21, left + (319), 21);

                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left, 11, left + 319, 11);
                painter->drawLine(left, 22, left + 319, 22);

                drawVerticalEmboss(left + 106, 1, 9, colorHilite, colorShadow, colorShadow, painter);

                drawVerticalEmboss(left + 213, 1, 9, colorHilite, colorShadow, colorShadow, painter);
                drawVerticalEmboss(left + 45, 22, 10, colorHilite, colorShadow, colorShadow, painter);


                //far left emboss
                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 1, left + 1, 9);
                m_pen.setColor(colorHilite);
                painter->setPen(m_pen);
                painter->drawLine(left + 1, 12, left + 1, 32);

                //far right emboss
                m_pen.setColor(colorShadow);
                painter->setPen(m_pen);
                painter->drawLine(left + 320, 0, left + (320), 32);
            }


            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("game music creator"))
            {
                m_height = 27;


                int left = 0;

                QRectF source(0, 0, 320, 27);
                QRectF target(left, 0, 320, 27);

                QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" +
                    QDir::separator() + "trackerview" + QDir::separator() + "gmc_top.png";
                QImage imageTop(imagepath);
                painter->drawImage(target, imageTop, source);

                painter->setPen(QColor(173, 138, 115));
                drawText(QString("%1").arg(m_currentPosition, 2, 10, QChar('0')), painter, left + 88, 14);
                drawText(QString("%1").arg(m_currentPattern, 2, 10, QChar('0')), painter, left + 194, 14);
                drawText(QString("%1").arg(m_info->numOrders, 2, 10, QChar('0')), painter, left + 300, 14);
            }
        }
        else
        {
            m_height = 0;
        }
    }

    m_trackerview->paintAbove(painter, height, currentRow);


    if (m_render && m_renderVUMeter)
    {
        SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);

        int HEIGHT = 47;
        int WIDTH = 10;
        int LEFT_OFFSET = 55;
        int VUMETER_OFFSET = 72;
        int HILIGHT_WIDTH = 2;
        TOP_OFFSET = -6;
        if (QString(m_info->fileformat.c_str()).toLower().startsWith("taketracker"))
        {
            HEIGHT = 48;
            WIDTH = 22;
            LEFT_OFFSET = 61;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("ultimate") ||
            QString(m_info->fileformat.c_str()).toLower().startsWith("soundtracker"))
        {
            HEIGHT = 48;
            WIDTH = 8;
            LEFT_OFFSET = 56;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("startrekker"))
        {
            TOP_OFFSET = -5;
        }
        else if (QString(m_info->fileformat.c_str()) == "AHX")
        {
            WIDTH = 16;
            LEFT_OFFSET = 58;
            TOP_OFFSET = -9;
        }
        else if (QString(m_info->fileformat.c_str()) == "HivelyTracker")
        {
            HEIGHT = 64;
            WIDTH = 32;
            LEFT_OFFSET = 67;
            VUMETER_OFFSET = 120;
            HILIGHT_WIDTH = 1;
            TOP_OFFSET = -13;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0"))
        {
            WIDTH = 24;
            HEIGHT = 128;
            LEFT_OFFSET = 152;
            HILIGHT_WIDTH = 4;
            VUMETER_OFFSET = 144;
            TOP_OFFSET = -6;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1"))
        {
            WIDTH = 24;
            HEIGHT = 128;
            LEFT_OFFSET = 152;
            HILIGHT_WIDTH = 4;
            VUMETER_OFFSET = 144;
            TOP_OFFSET = -21;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
        {
            WIDTH = 24;
            HEIGHT = 128;
            LEFT_OFFSET = 152;
            HILIGHT_WIDTH = 4;
            VUMETER_OFFSET = 144;
            TOP_OFFSET = -6;
        }
        else if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (patt"))
        {
            WIDTH = 12;
            HEIGHT = 94;
            LEFT_OFFSET = 90;
            HILIGHT_WIDTH = 2;
            VUMETER_OFFSET = 120;
            TOP_OFFSET = -18;
        }


        maxHeight = HEIGHT;


        if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0") ||
            QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1") ||
            QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
        {
            painter->translate(0, (height) - maxHeight + TOP_OFFSET);
        }
        else
        {
            painter->translate(0, (height / 2) - maxHeight + TOP_OFFSET);
        }

        for (int unsigned i = 0; i < m_info->numChannels; i++)
        {
            unsigned char volume = static_cast<unsigned char>(((m_info->modVUMeters[i] / 64.0) + 0.005) * 100);
            //volume is between 0-100. 0.005 for rounding


            int vumeterCurrentHeight = (volume * HEIGHT / 100);

            int vuWidth = WIDTH;
            int xPos = (i * VUMETER_OFFSET) + LEFT_OFFSET;


            QRect rectL(xPos, maxHeight - vumeterCurrentHeight, vuWidth, vumeterCurrentHeight);
            QRect rectLHiLite(xPos, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH, vumeterCurrentHeight);
            QRect rectLDark(xPos + vuWidth - HILIGHT_WIDTH, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH,
                            vumeterCurrentHeight);


            QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, maxHeight));
            QLinearGradient linearGradHiLite(QPointF(0, 0), QPointF(0, maxHeight));
            QLinearGradient linearGradDark(QPointF(0, 0), QPointF(0, maxHeight));


            if (QString(m_info->fileformat.c_str()).toLower().startsWith("protracker mod (flt4"))
            {
                //main color
                linearGrad.setColorAt(0, QColor(0, 0, 206).rgb());
                linearGrad.setColorAt(1, QColor(33, 222, 222).rgb());

                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(0, 0, 239).rgb());
                linearGradHiLite.setColorAt(1, QColor(49, 255, 255).rgb());

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(0, 0, 156).rgb());
                linearGradDark.setColorAt(1, QColor(16, 189, 189).rgb());

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("taketracker"))
            {
                painter->setCompositionMode(QPainter::CompositionMode_Lighten);
                for (int i = 0; i < 6; i++)
                {
                    QRect rectL(xPos + (i * 4), maxHeight - vumeterCurrentHeight, 2, vumeterCurrentHeight);
                    painter->fillRect(rectL, QBrush(QColor(255, 255, 0, 170)));
                }
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("noise") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("chiptracker") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("ice tracker") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("mnemotron") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("his master"))
            {
                //main color
                linearGrad.setColorAt(0, QColor(189, 16, 0).rgb()); //red
                linearGrad.setColorAt(0.11, QColor(189, 50, 0).rgb()); //red
                linearGrad.setColorAt(0.36, QColor(189, 255, 0).rgb()); //yellow
                linearGrad.setColorAt(0.81, QColor(0, 255, 0).rgb()); // green
                linearGrad.setColorAt(0.87, QColor(0, 227, 0).rgb()); // green
                linearGrad.setColorAt(0.95, QColor(0, 169, 0).rgb()); // green


                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(255, 16, 0).rgb()); //red
                linearGradHiLite.setColorAt(0.11, QColor(255, 50, 0).rgb()); //red
                linearGradHiLite.setColorAt(0.36, QColor(255, 255, 0).rgb()); //yellow
                linearGradHiLite.setColorAt(0.81, QColor(66, 255, 0).rgb()); // green
                linearGradHiLite.setColorAt(1, QColor(0, 239, 0).rgb()); // dark green

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(115, 16, 0).rgb()); //red
                linearGradDark.setColorAt(0.11, QColor(111, 50, 0).rgb()); //red
                linearGradDark.setColorAt(0.36, QColor(115, 255, 0).rgb()); //yellow
                linearGradDark.setColorAt(0.74, QColor(0, 218, 0).rgb()); // green
                linearGradDark.setColorAt(0.85, QColor(0, 166, 0).rgb()); // green
                linearGradDark.setColorAt(0.93, QColor(0, 105, 0).rgb()); // green
                linearGradDark.setColorAt(1, QColor(0, 96, 0).rgb()); // dark green

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("pt3.6") || QString(
                m_info->fileformat.c_str()).toLower().startsWith("pt4"))
            {
                //main color
                linearGrad.setColorAt(0, QColor(0, 0, 0).rgb()); //black
                linearGrad.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
                linearGrad.setColorAt(0.02, QColor(75, 0, 0).rgb()); //red
                linearGrad.setColorAt(0.07, QColor(136, 17, 0).rgb()); //red
                linearGrad.setColorAt(0.12, QColor(170, 34, 0).rgb()); //red
                linearGrad.setColorAt(0.22, QColor(238, 102, 0).rgb()); //red
                linearGrad.setColorAt(0.39, QColor(255, 238, 0).rgb()); //yellow
                linearGrad.setColorAt(0.48, QColor(238, 255, 0).rgb()); //yellow
                linearGrad.setColorAt(0.64, QColor(153, 238, 0).rgb()); // green
                linearGrad.setColorAt(1, QColor(17, 85, 0).rgb()); // green


                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(0, 0, 0).rgb()); //black
                linearGradHiLite.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
                linearGradHiLite.setColorAt(0.02, QColor(136, 51, 51).rgb()); //red
                linearGradHiLite.setColorAt(0.07, QColor(187, 68, 51).rgb()); //red
                linearGradHiLite.setColorAt(0.12, QColor(221, 85, 51).rgb()); //red
                linearGradHiLite.setColorAt(0.22, QColor(255, 153, 51).rgb()); //red
                linearGradHiLite.setColorAt(0.39, QColor(255, 255, 51).rgb()); //yellow
                linearGradHiLite.setColorAt(0.48, QColor(255, 255, 51).rgb()); //yellow
                linearGradHiLite.setColorAt(0.64, QColor(204, 255, 51).rgb()); // green
                linearGradHiLite.setColorAt(1, QColor(68, 136, 51).rgb()); // green

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(0, 0, 0).rgb()); //black
                linearGradDark.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
                linearGradDark.setColorAt(0.02, QColor(34, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.07, QColor(85, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.12, QColor(119, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.22, QColor(187, 51, 0).rgb()); //red
                linearGradDark.setColorAt(0.39, QColor(204, 170, 0).rgb()); //yellow
                linearGradDark.setColorAt(0.48, QColor(187, 204, 0).rgb()); //yellow
                linearGradDark.setColorAt(0.64, QColor(102, 187, 51).rgb()); // green
                linearGradDark.setColorAt(1, QColor(0, 34, 0).rgb()); // green

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
            {
                //main color
                linearGrad.setColorAt(0, QColor(187, 187, 187).rgb());
                linearGrad.setColorAt(0.045, QColor(187, 187, 187).rgb());
                linearGrad.setColorAt(0.04501, QColor(255, 34, 17).rgb()); //red
                linearGrad.setColorAt(0.46, QColor(255, 255, 51).rgb()); //yellow
                linearGrad.setColorAt(0.75, QColor(0, 255, 0).rgb()); // green
                linearGrad.setColorAt(1, QColor(0, 136, 0).rgb()); // dark green


                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(QColor(187, 187, 187)));
                painter->fillRect(rectLDark, QBrush(QColor(187, 187, 187)));
                //painter->fillRect(0,0,width(),height(),QColor(255,0,0));
            }
            else if (QString(m_info->fileformat.c_str()).toLower().startsWith("ultimate") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("soundtracker") ||
                QString(m_info->fileformat.c_str()).toLower().startsWith("converted st2"))
            {
                //main color
                linearGrad.setColorAt(0, QColor(156, 0, 0).rgb()); //red
                linearGrad.setColorAt(0.250000000, QColor(156, 0, 0).rgb()); //red
                linearGrad.setColorAt(0.250000001, QColor(0, 140, 0).rgb()); //green
                linearGrad.setColorAt(1, QColor(0, 140, 0).rgb()); // green

                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(189, 0, 0).rgb()); //red
                linearGradHiLite.setColorAt(0.250000000, QColor(189, 0, 0).rgb()); //red
                linearGradHiLite.setColorAt(0.250000001, QColor(0, 173, 0).rgb()); //green
                linearGradHiLite.setColorAt(1, QColor(0, 173, 0).rgb()); // green

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(115, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.250000000, QColor(115, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.250000001, QColor(0, 99, 0).rgb()); //green
                linearGradDark.setColorAt(1, QColor(0, 99, 0).rgb()); // green

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
            else if (QString(m_info->fileformat.c_str()) == "AHX")
            {
                //painter->setCompositionMode(QPainter::CompositionMode_Lighten);
                QColor col(255, 255, 255);
                col.setHsl(static_cast<int>(m_fColorHueCounter), static_cast<int>(m_fColorSaturationCounter), 127);
                painter->fillRect(rectL, QBrush(col));
                m_fColorHueCounter += 1;
                m_fColorLightnessCounter += m_fColorLightnessdirection;
                if (m_fColorLightnessCounter > 255 or m_fColorLightnessCounter < 0)
                {
                    m_fColorLightnessdirection *= -1;
                }
                m_fColorSaturationCounter += m_fColorSaturationdirection;
                if (m_fColorSaturationCounter > 255 or m_fColorSaturationCounter < 0)
                {
                    m_fColorSaturationdirection *= -1;
                }
                if (m_fColorHueCounter > 255) m_fColorHueCounter = 0;
            }
            else if (QString(m_info->fileformat.c_str()) == "HivelyTracker")
            {
                QLinearGradient linearGrad(QPointF(0, maxHeight - vumeterCurrentHeight), QPointF(0, maxHeight));
                QLinearGradient linearGradHiLite(QPointF(0, maxHeight - vumeterCurrentHeight), QPointF(0, maxHeight));
                QLinearGradient linearGradDark(QPointF(0, maxHeight - vumeterCurrentHeight), QPointF(0, maxHeight));


                linearGrad.setColorAt(0, QColor(255, 255, 255).rgb());
                linearGrad.setColorAt(0.02, QColor(255, 255, 255).rgb());
                linearGrad.setColorAt(0.02, QColor(228, 239, 249).rgb());
                linearGrad.setColorAt(0.12, QColor(227, 238, 249).rgb());
                linearGrad.setColorAt(0.15, QColor(230, 240, 250).rgb());
                linearGrad.setColorAt(0.5, QColor(165, 199, 232).rgb());
                linearGrad.setColorAt(1, QColor(88, 114, 139).rgb());

                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(255, 255, 255).rgb());
                linearGradHiLite.setColorAt(1, QColor(144, 175, 206).rgb());

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(170, 170, 170).rgb());
                linearGradDark.setColorAt(1, QColor(26, 45, 65).rgb());

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
            else //protracker
            {
                //main color
                linearGrad.setColorAt(0, QColor(255, 16, 0).rgb()); //red
                linearGrad.setColorAt(0.13, QColor(255, 48, 0).rgb()); //red
                linearGrad.setColorAt(0.4, QColor(255, 255, 0).rgb()); //yellow
                linearGrad.setColorAt(1, QColor(0, 255, 0).rgb()); // green


                //hilight color (left)
                linearGradHiLite.setColorAt(0, QColor(255, 69, 49).rgb()); //red
                linearGradHiLite.setColorAt(0.13, QColor(255, 104, 52).rgb()); //red
                linearGradHiLite.setColorAt(0.34, QColor(255, 255, 52).rgb()); //yellow
                linearGradHiLite.setColorAt(0.54, QColor(255, 255, 52).rgb()); //yellow
                linearGradHiLite.setColorAt(1, QColor(49, 255, 49).rgb()); // green

                //dark color (right)
                linearGradDark.setColorAt(0, QColor(206, 0, 0).rgb()); //red
                linearGradDark.setColorAt(0.21, QColor(206, 47, 0).rgb()); //red
                linearGradDark.setColorAt(0.4, QColor(204, 204, 0).rgb()); //yellow
                linearGradDark.setColorAt(1, QColor(0, 187, 0).rgb()); // green

                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
        }
    }


    //move painter back up (was moved down when painted vumeters)
    if (m_renderVUMeter)
    {
        if (QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd0") ||
            QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd1") ||
            QString(m_info->fileformat.c_str()).toLower().startsWith("octamed (mmd2"))
        {
            painter->translate(0, -1 * ((height) - maxHeight + TOP_OFFSET));
        }
        else
        {
            painter->translate(0, -1 * ((height / 2) - maxHeight + TOP_OFFSET));
        }
    }

    for (int i = 0; i < m_info->numChannels; i++)
    {
        if (SoundManager::getInstance().isChannelMuted(i))
        {
            //draw background with opacity when enabled/disabled channels
            QBrush brush(m_trackerview->colorBackground());
            int x = m_trackerview->channelStart() + (i * m_trackerview->channelWidth()) + (i * m_trackerview->
                channelxSpace());
            int y = m_trackerview->topHeight();
            int w;
            if (i == 0) //first channel
            {
                w = m_trackerview->channelFirstWidth();
            }
            else if (i == m_info->numChannels - 1) //last channel
            {
                if (m_trackerview->channelFirstWidth() != m_trackerview->channelWidth())
                {
                    x = x - (m_trackerview->channelWidth() - m_trackerview->channelFirstWidth());
                }

                w = m_trackerview->channelLastWidth();
            }
            else
            {
                if (m_trackerview->channelFirstWidth() != m_trackerview->channelWidth())
                {
                    x = x - (m_trackerview->channelWidth() - m_trackerview->channelFirstWidth());
                }
                w = m_trackerview->channelWidth();
            }
            int h = height - y - m_trackerview->bottomFrameHeight();
            painter->fillRect(QRect(x, y, w, h), brush);
        }
    }
}


void Tracker::drawVerticalEmboss(int xPos, int yPos, int height, QColor hilite, QColor shadow, QColor base,
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
