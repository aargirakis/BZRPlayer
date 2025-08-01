#include "tracker.h"
#include <plugins.h>
#include "mainwindow.h"
#include "qevent.h"
#include <QDir>
#include <QApplication>
#include "soundmanager.h"
#include "various.h"
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
    QString fontDir = dataPath + RESOURCES_DIR + "/trackerview/fonts" +
        QDir::separator();
    QFontDatabase::addApplicationFont(fontDir + "DejaVuSansMono.ttf");
}

void Tracker::init()
{
    m_info = SoundManager::getInstance().m_Info1;

    if (m_info == nullptr)
    {
        m_trackerview = nullptr;
        return;
    }

    if (m_info->plugin != PLUGIN_libxmp && m_info->plugin != PLUGIN_hivelytracker && m_info->plugin != PLUGIN_libopenmpt
        && m_info->plugin != PLUGIN_sunvox_lib)
    {
        m_trackerview = nullptr;
        return;
    }

    m_fColorHueCounter = 0;
    m_fColorLightnessCounter = 0;
    m_fColorLightnessdirection = 0.3f;
    m_fColorSaturationCounter = 0;
    m_fColorSaturationdirection = 0.75f;

    QString fileFormat = QString::fromStdString(m_info->fileformat).toLower();
    if (fileFormat.startsWith("SunVox"))
    {
        m_trackerview = new SoundFXPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (6chn") ||
            fileFormat.startsWith("protracker mod (8chn") ||
            fileFormat.startsWith("protracker mod (16ch"))
    {
        m_trackerview = new FastTracker1PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("soundfx"))
    {
        m_trackerview = new SoundFXPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("ice tracker"))
    {
        m_trackerview = new IceTrackerPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (patt"))
    {
        m_trackerview = new ProTracker36PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (flt4"))
    {
        m_trackerview = new StarTrekker13PatternView(this, m_info->numChannels);
    }
    else if ((fileFormat.startsWith("protracker") && !
                                                             fileFormat.startsWith("protracker xm")) ||
            fileFormat.startsWith("probably converted (m.k") ||
            fileFormat.startsWith("unknown/converted (m.k") ||
            fileFormat.startsWith("converted 15 ins") || fileFormat.startsWith("unknown or converted (M"))
    {
        m_trackerview = new ProTracker1PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("noise") || QString(m_info->fileformat.c_str()).
        toLower().startsWith("his master"))
    {
        m_trackerview = new NoiseTrackerPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("mnemotron"))
    {
        m_trackerview = new SoundTracker26PatternView(this, m_info->numChannels);
    }

    else if (fileFormat == "soundtracker")
    {
        m_trackerview = new UltimateSoundTrackerPatternView(this, m_info->numChannels);
    }

    else if (fileFormat.startsWith("game music creator"))
    {
        m_trackerview = new GameMusicCreatorPatternView(this, m_info->numChannels);
    }
    else if (QString(m_info->fileformat.c_str()) == "AHX")
    {
        m_trackerview = new AHXPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("chiptracker"))
    {
        m_trackerview = new ChipTrackerPatternView(this, m_info->numChannels);
    }

    else if (QString(m_info->fileformat.c_str()) == "HivelyTracker")
    {
        m_trackerview = new HivelyTrackerPatternView(this, m_info->numChannels);
    }

    else if (fileFormat.startsWith("scream tracker 3"))
    {
        m_trackerview = new ScreamTracker3PatternView(this, m_info->numChannels);
    }

    else if (fileFormat.startsWith("scream tracker 2"))
    {
        m_trackerview = new ScreamTracker2PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("fasttracker 2") ||
            fileFormat.startsWith("skale tracker xm") ||
            fileFormat.startsWith("madtracker 2.0 xm") ||
            fileFormat.endsWith("xm 1.04"))
    {
        if (m_info->numChannels > 6)
        {
            m_trackerview = new FastTracker2PatternView(this, m_info->numChannels);
        }
        else if (m_info->numChannels > 4)
        {
            m_trackerview = new FastTracker26ChanPatternView(this, m_info->numChannels);
        }
        else
        {
            m_trackerview = new FastTracker24ChanPatternView(this, m_info->numChannels);
        }
    }

    else if (fileFormat.startsWith("octamed (mmd0"))
    {
        m_trackerview = new MEDPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("impulse"))
    {
        m_trackerview = new ImpulseTrackerPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("composer 669"))
    {
        m_trackerview = new Composer669PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("ultratracker"))
    {
        m_trackerview = new UltraTrackerPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("digibooster pro"))
    {
        m_trackerview = new DigiBoosterProPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("digibooster"))
    {
        m_trackerview = new DigiBooster17PatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("oktalyzer"))
    {
        m_trackerview = new OktalyzerPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("octamed (mmd1"))
    {
        if (m_info->numChannels > 4)
        {
            m_trackerview = new OctaMEDPatternView(this, m_info->numChannels);
        }
        else
        {
            m_trackerview = new OctaMED44ChanPatternView(this, m_info->numChannels);
        }
    }
    else if (fileFormat.startsWith("octamed (mmd2"))
    {
        if (m_info->numChannels > 4)
        {
            m_trackerview = new OctaMED5ChanPatternView(this, m_info->numChannels);
        }
        else
        {
            m_trackerview = new OctaMED54ChanPatternView(this, m_info->numChannels);
        }
    }
    else if (fileFormat.startsWith("octamed (mmd3"))
    {
        m_trackerview = new OctaMEDSoundstudioPatternView(this, m_info->numChannels);
    }
    else if (fileFormat.startsWith("multitracker") || fileFormat.startsWith("mdx"))
    {
        m_trackerview = new MultiTrackerPatternView(this, m_info->numChannels);
    }
    else
    {
        m_trackerview = m_trackerview = nullptr;
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
        || m_info->plugin == PLUGIN_sunvox_lib)
    {
        SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN_INFO);
    }
}

void Tracker::drawPattern(QPainter* painter)
{
    if (m_trackerview==nullptr)
    {
        return;
    }
    int height = m_trackerview->height();
    int width = m_trackerview->width();


    unsigned int currentPattern = 0;
    unsigned int currentRow = 0;
    unsigned int currentPosition = 0;
    unsigned int currentSpeed = 0;
    unsigned int currentBPM = 0;
    bool isHivelyTracker = m_info->plugin == PLUGIN_hivelytracker;
    bool islibopenmpt = m_info->plugin == PLUGIN_libopenmpt;
    bool islibxmp = m_info->plugin == PLUGIN_libxmp;
    bool issunvox = m_info->plugin == PLUGIN_sunvox_lib;

    currentPosition = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODORDER);
    currentRow = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODROW);
    currentPattern = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN);
    currentSpeed = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_SPEED);
    currentBPM = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_BPM);


    bool updateHively = false;
    if (isHivelyTracker)
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


    if(currentRow!=m_currentRow || m_trackerview->m_renderVUMeter || updateHively)
    {
        //Revert painter scaling and fill background rect
        QRect rect(0, 0, painter->window().width() / m_scale, painter->window().height() / m_scale);
        painter->fillRect(rect, QColor(0, 0, 0));

        m_currentRow = currentRow;
        m_currentPattern = currentPattern; //used where one pattern per positions (mod/sfx)
        m_currentPosition = currentPosition;
        m_currentSpeed = currentSpeed;
        m_currentBPM = currentBPM;

        if (islibopenmpt || islibxmp)
        {
            m_info->modPatternRows = m_info->patterns[m_currentPattern].size() / m_info->numChannels;
        }

        if (isHivelyTracker)
        {
            m_currentTrackPositions = std::vector<unsigned char>(m_info->modTrackPositions.size());
            copy(m_info->modTrackPositions.begin(), m_info->modTrackPositions.end(), m_currentTrackPositions.begin());
            //used where multiple patterns positions per position (ahx)
        }

        unsigned int j = 0;
        unsigned int i = 0;
        unsigned int outerLoopBreakValue = 0;
        if (isHivelyTracker)
        {
            outerLoopBreakValue = m_info->modPatternRows;
        }

        else if (islibopenmpt || islibxmp)
        {
            i = 0;
            outerLoopBreakValue = m_info->patterns[m_currentPattern].size();
        }


        m_trackerview->paintBelow(painter, height, currentRow);


        int libxmpNotes = 0;
        int yPixelPosition = 0;
        const int yOffset = height / 2;
        const QFont& currentRowFont = m_trackerview->currentRowFont();
        const QFont& normalFont = m_trackerview->font();

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
                yPixelPosition += (currentRowFont.pixelSize() + m_trackerview->yOffsetRowAfter());
                yPixelPosition -= m_trackerview->yOffsetCurrentRowBefore();
                if (currentRow == 0)
                {
                    yPixelPosition += m_trackerview->yOffsetCurrentRowBefore();
                    yPixelPosition -= (m_trackerview->fontHeight() + m_trackerview->yOffsetRowAfter());
                }
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
                                                                                                           (normalFont.pixelSize() + m_trackerview->yOffsetRowAfter()));

                if (j > currentRow && currentRowFont.pixelSize() > normalFont.pixelSize())
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
                painter->setPen(colorDefault);
            }


            int fontWidth = m_trackerview->fontWidth();

            int numPixels = 0;


            if (!m_trackerview->rowStart().isEmpty())
            {
                painter->setPen(colorRowNumber);
                m_trackerview->drawText(m_trackerview->rowStart(), painter, m_trackerview->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,m_trackerview->bitmapFont());
            }
            numPixels += (m_trackerview->rowStart().length() * fontWidth);


            painter->setPen(colorRowNumber);
            if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->bitmapFontRownumber().
                    m_bitmapFontPath)
            {
                fontWidth = m_trackerview->fontWidth();
                m_trackerview->drawText(strRowNumber, painter, (m_trackerview->xOffsetRow() + numPixels), yOffset + yPixelPosition,m_trackerview->currentRowBitmapFont());
            }
            else
            {
                fontWidth = m_trackerview->fontWidthRownumber();
                m_trackerview->drawText(strRowNumber, painter, (m_trackerview->xOffsetRow() + numPixels), yOffset + yPixelPosition,m_trackerview->bitmapFontRownumber());
            }






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
                if (isHivelyTracker)
                {
                    k = m_info->modTrackPositions.at(chan) * m_info->modPatternRows;
                }
                else if (islibopenmpt || islibxmp || issunvox)
                {
                    k = chan;
                }

                BaseRow* row;
                if (islibopenmpt || islibxmp || issunvox)
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
                    m_trackerview->drawText(channelSeparator, painter, (m_trackerview->xOffsetRow() + numPixels),
                     yOffset + yPixelPosition,m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    m_trackerview->drawText(channelSeparator, painter, (m_trackerview->xOffsetRow() + numPixels),
                     yOffset + yPixelPosition,m_trackerview->bitmapFontRownumber());
                    fontWidth = m_trackerview->fontWidthRownumber();
                }


                numPixels += channelSeparator.length() * fontWidth;
                numPixels += strRowNumberMultiple.length() * fontWidth;
                fontWidth = m_trackerview->fontWidth();

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

                if (j == currentRow)
                {
                    m_trackerview->drawText(strNote, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,m_trackerview->currentRowBitmapFont());
                }
                else
                {
                    m_trackerview->drawText(strNote, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,m_trackerview->bitmapFont());
                }


                numPixels += strNote.length() * fontWidth;
                fontWidth = m_trackerview->fontWidthSeparatorNote();
                m_trackerview->drawText(m_trackerview->separatorNote(), painter, m_trackerview->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition, m_trackerview->bitmapFont());

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

                BitmapFont font;
                if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontInstrument().m_bitmapFontPath)
                {
                    font = m_trackerview->currentRowBitmapFont();
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    font = m_trackerview->bitmapFontInstrument();
                    fontWidth = m_trackerview->fontWidthInstrument();
                }

                m_trackerview->drawText(instrument, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                numPixels += instrument.length() * fontWidth;
                m_trackerview->drawText(m_trackerview->separatorInstrument(), painter, m_trackerview->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,font);
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
                m_trackerview->drawText(volume, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                numPixels += volume.length() * fontWidth;
                m_trackerview->drawText(m_trackerview->separatorVolume(), painter, m_trackerview->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,font);
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
                    BitmapFont font;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontEffects().m_bitmapFontPath)
                    {
                        font = m_trackerview->currentRowBitmapFont();
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font = m_trackerview->bitmapFontEffects();
                        fontWidth = m_trackerview->fontWidthEffects();
                    }

                    m_trackerview->drawText(effect, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                    numPixels += effect.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorEffect(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font);
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
                    BitmapFont font1;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontParameters().m_bitmapFontPath)
                    {
                        font = m_trackerview->currentRowBitmapFont();
                        m_trackerview->drawText(parameter, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font1);
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font = m_trackerview->bitmapFontParameters();
                        m_trackerview->drawText(parameter, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font1);
                        fontWidth = m_trackerview->fontWidthParameters();
                    }

                    numPixels += parameter.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorParameter(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font);
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
                    BitmapFont font2;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontEffects().m_bitmapFontPath)
                    {
                        font = m_trackerview->currentRowBitmapFont();
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font = m_trackerview->bitmapFontEffects();
                        fontWidth = m_trackerview->fontWidthEffects();
                    }
                    m_trackerview->drawText(effect2, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font2);
                    numPixels += effect2.length() * fontWidth,font2;
                    m_trackerview->drawText(m_trackerview->separatorEffect2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font2);
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
                    BitmapFont font0;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontParameters().m_bitmapFontPath)
                    {
                        font0 = m_trackerview->currentRowBitmapFont();
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font0 = m_trackerview->bitmapFontParameters();
                        fontWidth = m_trackerview->fontWidthParameters();
                    }
                    m_trackerview->drawText(parameter2, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font0);
                    numPixels += parameter2.length() * fontWidth,font0;
                    m_trackerview->drawText(m_trackerview->separatorParameter2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font0);
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
                    BitmapFont font00;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontEffects().m_bitmapFontPath)
                    {
                        font00 = m_trackerview->currentRowBitmapFont();
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font00 = m_trackerview->bitmapFontEffects();
                        fontWidth = m_trackerview->fontWidthEffects();
                    }
                    m_trackerview->drawText(effect, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font00);
                    numPixels += effect.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorEffect(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font00);
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
                    m_trackerview->drawText(effect2, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font00);
                    numPixels += effect2.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorEffect2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font00);
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
                    BitmapFont font4;
                    if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                            bitmapFontParameters().m_bitmapFontPath)
                    {
                        font4 = m_trackerview->currentRowBitmapFont();
                        fontWidth = m_trackerview->fontWidth();
                    }
                    else
                    {
                        font4 = m_trackerview->bitmapFontParameters();
                        fontWidth = m_trackerview->fontWidthParameters();
                    }
                    m_trackerview->drawText(parameter, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font4);
                    numPixels += parameter.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorParameter(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font4);
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
                    m_trackerview->drawText(parameter2, painter, m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,font4);
                    numPixels += parameter2.length() * fontWidth;
                    m_trackerview->drawText(m_trackerview->separatorParameter2(), painter, m_trackerview->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font4);
                    numPixels += m_trackerview->separatorParameter2().length() * fontWidth;
                }
            }


            if (m_trackerview->rowNumbersLastChannelEnabled())
            {
                if (j == currentRow && m_trackerview->bitmapFont().m_bitmapFontPath == m_trackerview->
                        bitmapFontRownumber().m_bitmapFontPath)
                {
                    m_trackerview->drawText(m_trackerview->separatorRowNumberLast() + row, painter,
                             m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,m_trackerview->currentRowBitmapFont());
                    fontWidth = m_trackerview->fontWidth();
                }
                else
                {
                    m_trackerview->drawText(m_trackerview->separatorRowNumberLast() + row, painter,
                             m_trackerview->xOffsetRow() + numPixels, yOffset + yPixelPosition,m_trackerview->bitmapFontRownumber());
                    fontWidth = m_trackerview->fontWidthRownumber();
                }
                painter->setPen(colorRowNumber);

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
                                  yOffset + yPixelPosition + m_trackerview->yOffsetRowHighlight(),
                                  m_trackerview->xOffsetRow() + numPixels - (m_trackerview->
                                          highlightBackgroundOffset()), 1, QColor(0, 0, 0));
            }
            j++;
            if (islibopenmpt || islibxmp || issunvox)
            {
                i += m_info->numChannels;
            }
            else if (isHivelyTracker)
            {
                i++;
            }
        }



    }

    m_trackerview->paintAbove(painter, height, currentRow);



}
void Tracker::drawTop(QPainter* painter)
{
    m_trackerview->paintTop(painter,m_info,m_currentPattern, m_currentPosition, m_currentSpeed,m_currentBPM, m_currentRow);
}
void Tracker::drawVUMeters(QPainter* painter)
{
    int height = m_trackerview->height();
    int width = m_trackerview->width();
    if (m_trackerview!=nullptr && m_trackerview->m_renderVUMeter)
    {
        SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);

        int HEIGHT = m_trackerview->m_vumeterHeight;
        int WIDTH = m_trackerview->m_vumeterWidth;
        int LEFT_OFFSET = m_trackerview->m_vumeterLeftOffset;
        int VUMETER_OFFSET = m_trackerview->m_vumeterOffset;
        int HILIGHT_WIDTH = m_trackerview->m_vumeterHilightWidth;
        TOP_OFFSET = m_trackerview->m_vumeterTopOffset;

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


            QLinearGradient linearGrad = m_trackerview->m_linearGrad;
            QLinearGradient linearGradHiLite = m_trackerview->m_linearGradHiLite;
            QLinearGradient linearGradDark = m_trackerview->m_linearGradDark;

            if (QString(m_info->fileformat.c_str()) == "AHX")
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
            else
            {


                painter->fillRect(rectL, QBrush(linearGrad));
                painter->fillRect(rectLHiLite, QBrush(linearGradHiLite));
                painter->fillRect(rectLDark, QBrush(linearGradDark));
            }
        }
    }


    //move painter back up (was moved down when painted vumeters)
    if (m_trackerview->m_renderVUMeter)
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

    m_trackerview->updateEnabledChannels(height, painter);
}
void Tracker::paint(QPainter* painter, QPaintEvent* event)
{
    if (m_trackerview==nullptr)
        return;

    QSize renderSize = event->rect().size();
    bool sizeChanged = (renderSize != m_lastBufferSize);

    unsigned int currentPattern = 0;
    unsigned int currentRow = 0;
    unsigned int currentPosition = 0;
    unsigned int currentSpeed = 0;
    unsigned int currentBPM = 0;

    unsigned int currentSample = m_trackerview->getCurrentSample();

    unsigned int previousOldSample = m_trackerview->getCurrentSample();
    bool updateSample= false;
    if(m_currentSample != previousOldSample)
    {
        m_currentSample=m_trackerview->getCurrentSample();
        updateSample=true;
    }

    currentPosition = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODORDER);
    currentRow = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODROW);
    currentPattern = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN);
    currentSpeed = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_SPEED);
    currentBPM = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_BPM);

    bool trackerToUpdate;
    trackerToUpdate = currentPosition!=m_currentPositionBuffer || currentRow!=m_currentRowBuffer ||
            m_currentPattern!=m_currentPatternBuffer;


    bool topToUpdate;
    topToUpdate = currentPosition!=m_currentPositionBuffer  ||
                      m_currentPattern!=m_currentPatternBuffer || m_currentSpeed!=m_currentSpeedBuffer || m_currentBPM!=m_currentBPMBuffer || updateSample;

    if (sizeChanged || trackerToUpdate)
    {
        m_currentPositionBuffer = currentPosition;
        m_currentRowBuffer = currentRow;
        m_currentPatternBuffer = currentPattern;
        m_lastBufferSize = renderSize;
        m_backBuffer = QPixmap(renderSize);
        m_backBuffer.fill(Qt::black);

        QPainter bufferPainter(&m_backBuffer);

        float scaleX = float(renderSize.width()) / float(m_trackerview->width());
        float scaleY = float(renderSize.height()) / float(m_trackerview->height());
        m_scale = min(scaleX, scaleY);
        bufferPainter.scale(m_scale, m_scale);
        drawPattern(&bufferPainter);
    }

    if (m_lastVuSize != renderSize || m_trackerview->m_renderVUMeter)
    {
        m_lastVuSize = renderSize;
        m_vuBuffer = QPixmap(renderSize);
        m_vuBuffer.fill(Qt::transparent);

        QPainter vuPainter(&m_vuBuffer);
        vuPainter.setCompositionMode(QPainter::CompositionMode_Source);
        vuPainter.setRenderHint(QPainter::Antialiasing, false);
        vuPainter.scale(m_scale, m_scale);
        drawVUMeters(&vuPainter);  // this only draws the VUs
    }
    // Repaint Top bar layer
    if ((renderSize != m_lastTopBarSize || topToUpdate) && m_trackerview->m_renderTop)
    {
        m_currentPositionBuffer = currentPosition;
        m_currentPatternBuffer = currentPattern;
        m_currentSpeedBuffer = currentSpeed;
        m_currentBPMBuffer = currentBPM;
        m_lastTopBarSize = renderSize;
        m_topBarBuffer = QPixmap(renderSize);
        m_topBarBuffer.fill(Qt::transparent);

        QPainter topPainter(&m_topBarBuffer);
        topPainter.scale(m_scale, m_scale);
        drawTop(&topPainter);
    }

    // Composite all layers to screen
    painter->drawPixmap(0, 0, m_backBuffer);
    painter->drawPixmap(0, 0, m_vuBuffer);
    painter->drawPixmap(0, 0, m_topBarBuffer);
}





