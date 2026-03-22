#include <QDir>
#include <QFontDatabase>
#include "mainwindow.h"
#include "patternview/AHXPatternView.h"
#include "patternview/ChipTrackerPatternView.h"
#include "patternview/Composer669PatternView.h"
#include "patternview/DigiBooster17PatternView.h"
#include "patternview/DigiBoosterProPatternView.h"
#include "patternview/FastTracker1PatternView.h"
#include "patternview/FastTracker2PatternView.h"
#include "patternview/FastTracker24ChanPatternView.h"
#include "patternview/FastTracker26ChanPatternView.h"
#include "patternview/GameMusicCreatorPatternView.h"
#include "patternview/HivelyTrackerPatternView.h"
#include "patternview/IceTrackerPatternView.h"
#include "patternview/ImpulseTrackerPatternView.h"
#include "patternview/MEDPatternView.h"
#include "patternview/MultiTrackerPatternView.h"
#include "patternview/NoiseTrackerPatternView.h"
#include "patternview/OctaMED44ChanPatternView.h"
#include "patternview/OctaMED54ChanPatternView.h"
#include "patternview/OctaMED5ChanPatternView.h"
#include "patternview/OctaMEDPatternView.h"
#include "patternview/OctaMEDSoundstudioPatternView.h"
#include "patternview/OktalyzerPatternView.h"
#include "patternview/ProTracker1PatternView.h"
#include "patternview/ProTracker36PatternView.h"
#include "patternview/ScreamTracker2PatternView.h"
#include "patternview/ScreamTracker3PatternView.h"
#include "patternview/SoundFXPatternView.h"
#include "patternview/SoundTracker26PatternView.h"
#include "patternview/StarTrekker13PatternView.h"
#include "patternview/UltimateSoundTrackerPatternView.h"
#include "patternview/UltraTrackerPatternView.h"
#include "plugins.h"
#include "soundmanager.h"
#include "tracker.h"

Tracker::Tracker()
{
    const QString fontDir = dataPath + RESOURCES_DIR + "/trackerview/fonts" +
        QDir::separator();
    QFontDatabase::addApplicationFont(fontDir + "DejaVuSansMono.ttf");
}

void Tracker::init()
{
    const auto &sm = SoundManager::getInstance();
    info = sm.info;

    if (info == nullptr)
    {
        trackerView = nullptr;
        return;
    }

    if (info->plugin != PLUGIN_libxmp && info->plugin != PLUGIN_hivelytracker &&
        info->plugin != PLUGIN_libopenmpt && info->plugin != PLUGIN_sunvox_lib)
    {
        trackerView = nullptr;
        return;
    }

    if (const QString fileFormat = QString::fromStdString(info->fileFormat).toLower();
        fileFormat.startsWith("SunVox"))
    {
        trackerView = new SoundFXPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (6chn") ||
            fileFormat.startsWith("protracker mod (8chn") ||
            fileFormat.startsWith("protracker mod (16ch"))
    {
        trackerView = new FastTracker1PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("soundfx"))
    {
        trackerView = new SoundFXPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("ice tracker"))
    {
        trackerView = new IceTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (patt"))
    {
        trackerView = new ProTracker36PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("protracker mod (flt4"))
    {
        trackerView = new StarTrekker13PatternView(this, info->numChannels);
    }
    else if ((fileFormat.startsWith("protracker") && !fileFormat.startsWith("protracker xm")) ||
            fileFormat.startsWith("probably converted (m.k") ||
            fileFormat.startsWith("unknown/converted (m.k") ||
            fileFormat.startsWith("converted 15 ins") || fileFormat.startsWith("unknown or converted (M"))
    {
        trackerView = new ProTracker1PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("noise") || QString(info->fileFormat.c_str()).
        toLower().startsWith("his master"))
    {
        trackerView = new NoiseTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("mnemotron"))
    {
        trackerView = new SoundTracker26PatternView(this, info->numChannels);
    }
    else if (fileFormat == "soundtracker")
    {
        trackerView = new UltimateSoundTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("game music creator"))
    {
        trackerView = new GameMusicCreatorPatternView(this, info->numChannels);
    }
    else if (QString(info->fileFormat.c_str()) == "AHX")
    {
        trackerView = new AHXPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("chiptracker"))
    {
        trackerView = new ChipTrackerPatternView(this, info->numChannels);
    }
    else if (QString(info->fileFormat.c_str()) == "HivelyTracker")
    {
        trackerView = new HivelyTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("scream tracker 3"))
    {
        trackerView = new ScreamTracker3PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("scream tracker 2"))
    {
        trackerView = new ScreamTracker2PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("fasttracker 2") ||
            fileFormat.startsWith("skale tracker xm") ||
            fileFormat.startsWith("madtracker 2.0 xm") ||
            fileFormat.endsWith("xm 1.04"))
    {
        if (info->numChannels > 6)
        {
            trackerView = new FastTracker2PatternView(this, info->numChannels);
        }
        else if (info->numChannels > 4)
        {
            trackerView = new FastTracker26ChanPatternView(this, info->numChannels);
        }
        else
        {
            trackerView = new FastTracker24ChanPatternView(this, info->numChannels);
        }
    }
    else if (fileFormat.startsWith("octamed (mmd0"))
    {
        trackerView = new MEDPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("impulse"))
    {
        trackerView = new ImpulseTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("composer 669"))
    {
        trackerView = new Composer669PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("ultratracker"))
    {
        trackerView = new UltraTrackerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("digibooster pro"))
    {
        trackerView = new DigiBoosterProPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("digibooster"))
    {
        trackerView = new DigiBooster17PatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("oktalyzer"))
    {
        trackerView = new OktalyzerPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("octamed (mmd1"))
    {
        if (info->numChannels > 4)
        {
            trackerView = new OctaMEDPatternView(this, info->numChannels);
        }
        else
        {
            trackerView = new OctaMED44ChanPatternView(this, info->numChannels);
        }
    }
    else if (fileFormat.startsWith("octamed (mmd2"))
    {
        if (info->numChannels > 4)
        {
            trackerView = new OctaMED5ChanPatternView(this, info->numChannels);
        }
        else
        {
            trackerView = new OctaMED54ChanPatternView(this, info->numChannels);
        }
    }
    else if (fileFormat.startsWith("octamed (mmd3"))
    {
        trackerView = new OctaMEDSoundstudioPatternView(this, info->numChannels);
    }
    else if (fileFormat.startsWith("multitracker") || fileFormat.startsWith("mdx"))
    {
        trackerView = new MultiTrackerPatternView(this, info->numChannels);
    }
    else
    {
        trackerView = nullptr;
        return;
    }

    m_currentTrackPositions = std::vector<unsigned char>();
    m_currentTrackPositions.clear();
    m_currentRow = 0;
    m_currentPattern = -1;
    m_currentPosition = -1;
    m_currentSpeed = -1;
    m_currentBPM = -1;

    if (info->plugin == PLUGIN_libopenmpt || info->plugin == PLUGIN_hivelytracker ||
        info->plugin == PLUGIN_libxmp || info->plugin == PLUGIN_sunvox_lib) {
        sm.getPosition(FMOD_TIMEUNIT_MODPATTERN_INFO);
    }
}

void Tracker::drawPattern(QPainter* painter, int visibleWidth, bool forceRedraw)
{
    if (trackerView==nullptr)
    {
        return;
    }

    int height = trackerView->height();

    bool isHivelytracker = info->plugin == PLUGIN_hivelytracker;
    bool islibopenmpt = info->plugin == PLUGIN_libopenmpt;
    bool isLibxmp = info->plugin == PLUGIN_libxmp;
    bool isSunvox = info->plugin == PLUGIN_sunvox_lib;

    const auto &sm = SoundManager::getInstance();

    unsigned int currentPosition = sm.getPosition(FMOD_TIMEUNIT_MODORDER);
    unsigned int currentRow = sm.getPosition(FMOD_TIMEUNIT_MODROW);
    unsigned int currentPattern = sm.getPosition(FMOD_TIMEUNIT_MODPATTERN);
    unsigned int currentSpeed = sm.getPosition(FMOD_TIMEUNIT_SPEED);
    unsigned int currentBPM = sm.getPosition(FMOD_TIMEUNIT_BPM);

    bool updateHivelytracker = false;

    if (isHivelytracker)
    {
        for (unsigned int v = 0; v < info->modTrackPositions.size(); v++)
        {
            if (m_currentTrackPositions.empty() || m_currentTrackPositions.size() != info->modTrackPositions.size())
            {
                updateHivelytracker = true;
                break;
            }

            if (m_currentTrackPositions[v] != info->modTrackPositions[v])
            {
                updateHivelytracker = true;
                break;
            }
        }
    }

    if(currentRow!=m_currentRow || trackerView->m_renderVuMeter || updateHivelytracker || forceRedraw)
    {
        // revert painter scaling and fill background rect
        QRect rect(0, 0, painter->window().width() / scale, painter->window().height() / scale);
        painter->fillRect(rect, QColor(0, 0, 0));

        m_currentRow = currentRow;
        m_currentPattern = currentPattern; // used where one pattern per positions (mod/sfx)
        m_currentPosition = currentPosition;
        m_currentSpeed = currentSpeed;
        m_currentBPM = currentBPM;

        if (islibopenmpt || isLibxmp)
        {
            info->modPatternRows = info->patterns[m_currentPattern].size() / info->numChannels;
        }

        if (isHivelytracker)
        {
            m_currentTrackPositions = std::vector<unsigned char>(info->modTrackPositions.size());
            copy(info->modTrackPositions.begin(), info->modTrackPositions.end(), m_currentTrackPositions.begin());
            // used where multiple patterns positions per position (ahx)
        }

        unsigned int j = 0;
        unsigned int i = 0;
        unsigned int outerLoopBreakValue = 0;

        if (isHivelytracker)
        {
            outerLoopBreakValue = info->modPatternRows;
        }

        else if (islibopenmpt || isLibxmp)
        {
            i = 0;
            outerLoopBreakValue = info->patterns[m_currentPattern].size();
        }

        trackerView->paintBelow(painter, height, currentRow);

        int yPixelPosition = 0;
        const int yOffset = height / 2;
        const QFont& currentRowFont = trackerView->currentRowFont();
        const QFont& normalFont = trackerView->font();

        while (i < outerLoopBreakValue)
        {
            QString row = trackerView->rowNumber(j);

            QString strNote;
            QString strRowNumber;
            QColor colorRowNumber = trackerView->colorRowNumber();
            QColor colorInstrument = trackerView->colorInstrument();
            QColor colorEffect = trackerView->colorEffect();
            QColor colorEffect2 = trackerView->colorEffect2();
            QColor colorVolume = trackerView->colorVolume();
            QColor colorParameter = trackerView->colorParameter();
            QColor colorParameter2 = trackerView->colorParameter2();
            QColor colorDefault = trackerView->colorDefault();

            QColor colorInstrumentAlt;
            QColor colorEffectAlt;
            QColor colorEffect2Alt;
            QColor colorVolumeAlt;
            QColor colorParameterAlt;
            QColor colorParameter2Alt;
            QColor colorDefaultAlt;
            QColor colorEmptyAlt;

            if (trackerView->rowHighlightForegroundFrequency())
            {
                if (!(j % trackerView->rowHighlightForegroundFrequency()))
                {
                    colorRowNumber = trackerView->colorRowHighlightForeground();
                    colorInstrument = trackerView->colorRowHighlightForeground();
                    colorEffect = trackerView->colorRowHighlightForeground();
                    colorEffect2 = trackerView->colorRowHighlightForeground();
                    colorVolume = trackerView->colorRowHighlightForeground();
                    colorParameter = trackerView->colorRowHighlightForeground();
                    colorParameter2 = trackerView->colorRowHighlightForeground();
                    colorDefault = trackerView->colorRowHighlightForeground();
                }
            }

            if (trackerView->colorRowNumberHighLightFrequency())
            {
                if (!(j % trackerView->colorRowNumberHighLightFrequency()))
                {
                    colorRowNumber = trackerView->colorRowNumberHighLight();
                }
            }

            strRowNumber = row + trackerView->separatorRowNumber();

            if (j == currentRow)
            {
                yPixelPosition += currentRowFont.pixelSize() + trackerView->yOffsetRowAfter();
                yPixelPosition -= trackerView->yOffsetCurrentRowBefore();

                if (currentRow == 0)
                {
                    yPixelPosition += trackerView->yOffsetCurrentRowBefore();
                    yPixelPosition -= trackerView->fontHeight() + trackerView->yOffsetRowAfter();
                }

                painter->setPen(trackerView->colorCurrentRowForeground());

                if (!trackerView->colorRowNumberCurrentRowEnabled())
                {
                    colorRowNumber = trackerView->colorCurrentRowForeground();
                }
                else
                {
                    colorRowNumber = trackerView->colorRowNumberCurrentRow();
                }

                colorInstrument = trackerView->colorCurrentRowForeground();
                colorEffect = trackerView->colorCurrentRowForeground();
                colorEffect2 = trackerView->colorCurrentRowForeground();
                colorVolume = trackerView->colorCurrentRowForeground();
                colorParameter = trackerView->colorCurrentRowForeground();
                colorParameter2 = trackerView->colorCurrentRowForeground();
                colorDefault = trackerView->colorCurrentRowForeground();

                colorInstrumentAlt = trackerView->colorCurrentRowForeground();
                colorEffectAlt = trackerView->colorCurrentRowForeground();
                colorEffect2Alt = trackerView->colorCurrentRowForeground();
                colorVolumeAlt = trackerView->colorCurrentRowForeground();
                colorParameterAlt = trackerView->colorCurrentRowForeground();
                colorParameter2Alt = trackerView->colorCurrentRowForeground();
                colorDefaultAlt = trackerView->colorCurrentRowForeground();

                if (trackerView->currentRowHighlightForegroundFrequency())
                {
                    if (!(j % trackerView->currentRowHighlightForegroundFrequency()))
                    {
                        colorRowNumber = trackerView->colorCurrentRowHighlightForeground();
                        colorInstrument = trackerView->colorCurrentRowHighlightForeground();
                        colorEffect = trackerView->colorCurrentRowHighlightForeground();
                        colorEffect2 = trackerView->colorCurrentRowHighlightForeground();
                        colorVolume = trackerView->colorCurrentRowHighlightForeground();
                        colorParameter = trackerView->colorCurrentRowHighlightForeground();
                        colorParameter2 = trackerView->colorCurrentRowHighlightForeground();
                        colorDefault = trackerView->colorCurrentRowHighlightForeground();
                    }
                }
            }
            else
            {
                yPixelPosition = (trackerView->fontHeight() + trackerView->yOffsetRowAfter()) * j
                                 - currentRow * (normalFont.pixelSize() + trackerView->yOffsetRowAfter());

                if (j > currentRow && currentRowFont.pixelSize() > normalFont.pixelSize())
                {
                    yPixelPosition += trackerView->fontHeight() + trackerView->yOffsetRowAfter();
                }

                if (j > currentRow)
                {
                    yPixelPosition += trackerView->yOffsetCurrentRowAfter();
                }

                if (j < currentRow)
                {
                    yPixelPosition += trackerView->yOffsetCurrentRowBefore();
                }

                painter->setPen(colorDefault);
            }

            int fontWidth = trackerView->fontWidth();
            int numPixels = 0;

            if (!trackerView->rowStart().isEmpty())
            {
                painter->setPen(colorRowNumber);
                trackerView->drawText(trackerView->rowStart(), painter, trackerView->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,trackerView->bitmapFont());
            }

            numPixels += trackerView->rowStart().length() * fontWidth;

            if (trackerView->patternNumberAtStart()) // only used for Oktalyzer
            {
                painter->setPen(colorRowNumber);
                trackerView->drawText(QString::asprintf("%02X", m_currentPattern) + "'", painter, trackerView->xOffsetRow() + numPixels,
                                        yOffset + yPixelPosition,trackerView->bitmapFont());
                numPixels += 3 * fontWidth;
            }

            painter->setPen(colorRowNumber);

            if (j == currentRow && trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontRowNumber().
                    m_bitmapFontPath)
            {
                fontWidth = trackerView->fontWidth();
                trackerView->drawText(strRowNumber, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,trackerView->currentRowBitmapFont());
            }
            else
            {
                fontWidth = trackerView->fontWidthRowNumber();
                trackerView->drawText(strRowNumber, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,trackerView->bitmapFontRowNumber());
            }

            numPixels += strRowNumber.length() * fontWidth + trackerView->xOffsetSeparatorRowNumber();

            const int viewRight = visibleWidth;

            const int channelWidth = trackerView->channelWidth() + trackerView->channelxSpace(); // total pixel width per channel
            const int firstChannelWidth = trackerView->channelFirstWidth();
            const int lastChannelWidth = trackerView->channelLastWidth();

            const int contentX0 = trackerView->xOffsetRow() + numPixels;
            int x = contentX0;

            int firstVisibleChannel = 0;
            int lastVisibleChannel = info->numChannels - 1;

            for (int i = 0; i < info->numChannels; ++i) {
                int chanWidth = i == 0
                                    ? firstChannelWidth
                                    : i == info->numChannels - 1
                                          ? lastChannelWidth
                                          : channelWidth;

                int xEnd = x + chanWidth;

                // viewLeft is hardcoded to 0: you may support horizontal scrolling later
                if (constexpr int viewLeft = 0; xEnd >= viewLeft) {
                    firstVisibleChannel = i;
                    break;
                }

                x = xEnd;
            }

            x = contentX0;

            for (int i = 0; i < info->numChannels; ++i) {
                int chanWidth = i == 0 ? firstChannelWidth : i == info->numChannels - 1
                                                                 ? lastChannelWidth
                                                                 : channelWidth;

                int xEnd = x + chanWidth;

                if (x >= viewRight) {
                    lastVisibleChannel = i - 1;
                    if (lastVisibleChannel < firstVisibleChannel) lastVisibleChannel = firstVisibleChannel;
                    break;
                }

                x = xEnd;
            }

            for (int chan = firstVisibleChannel; chan <= lastVisibleChannel; chan++)
            {
                bool alternateChannelColors = false;

                if (trackerView->alternateChannelColorsFrequency())
                {
                    if (!(chan % trackerView->alternateChannelColorsFrequency()) && j != currentRow)
                    {
                        alternateChannelColors = true;
                        colorInstrumentAlt = trackerView->colorInstrumentAlternate();
                        colorEffectAlt = trackerView->colorEffectAlternate();
                        colorEffect2Alt = trackerView->colorEffect2Alternate();
                        colorVolumeAlt = trackerView->colorVolumeAlternate();
                        colorParameterAlt = trackerView->colorParameterAlternate();
                        colorParameter2Alt = trackerView->colorParameter2Alternate();
                        colorDefaultAlt = trackerView->colorDefaultAlternate();
                        colorEmptyAlt = trackerView->colorEmptyAlternate();
                    }
                }

                fontWidth = trackerView->fontWidth();
                int k = 0;

                if (isHivelytracker)
                {
                    k = info->modTrackPositions.at(chan) * info->modPatternRows;
                }
                else if (islibopenmpt || isLibxmp || isSunvox)
                {
                    k = chan;
                }

                BaseRow* row;

                if (islibopenmpt || isLibxmp || isSunvox) {
                    const int rowIndex = j * info->numChannels + chan;
                    row = info->patterns[m_currentPattern][rowIndex];
                } else {
                    row = info->modRows[i + k];
                }

                QString instrument = trackerView->instrument(row);
                QString effect = trackerView->effect(row);
                QString parameter = trackerView->parameter(row);
                QString effect2 = trackerView->effect2(row);
                QString parameter2 = trackerView->parameter2(row);

                strNote = trackerView->note(row);

                QString volume = trackerView->volume(row);
                QString channelSeparator = trackerView->separatorChannel();

                if (chan == 0)
                {
                    channelSeparator = "";
                }

                QString strRowNumberMultiple = strRowNumber;

                if (!trackerView->rowNumbersEveryChannelEnabled() || chan == 0)
                {
                    strRowNumberMultiple = "";
                }

                if (j == currentRow &&
                    trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontRowNumber().m_bitmapFontPath)
                {
                    trackerView->drawText(channelSeparator, painter, trackerView->xOffsetRow() + numPixels,
                     yOffset + yPixelPosition,trackerView->currentRowBitmapFont());
                    fontWidth = trackerView->fontWidth();
                }
                else
                {
                    trackerView->drawText(channelSeparator, painter, trackerView->xOffsetRow() + numPixels,
                     yOffset + yPixelPosition,trackerView->bitmapFontRowNumber());
                    fontWidth = trackerView->fontWidthRowNumber();
                }

                numPixels += channelSeparator.length() * fontWidth;
                numPixels += strRowNumberMultiple.length() * fontWidth;
                fontWidth = trackerView->fontWidth();

                if (strNote == trackerView->emptyNote() && j != currentRow)
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(trackerView->colorEmpty());
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
                    trackerView->drawText(strNote, painter, trackerView->xOffsetRow() + numPixels,
                                          yOffset + yPixelPosition, trackerView->currentRowBitmapFont());
                }
                else
                {
                    trackerView->drawText(strNote, painter, trackerView->xOffsetRow() + numPixels,
                                          yOffset + yPixelPosition, trackerView->bitmapFont());
                }

                numPixels += strNote.length() * fontWidth;
                fontWidth = trackerView->fontWidthSeparatorNote();
                trackerView->drawText(trackerView->separatorNote(), painter, trackerView->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition, trackerView->bitmapFont());

                numPixels += trackerView->separatorNote().length() * fontWidth;

                if (instrument == trackerView->emptyInstrument() &&
                    !trackerView->noEmptyInstrumentColor() && j != currentRow) {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(trackerView->colorEmpty());
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

                if (j == currentRow &&
                    trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontInstrument().m_bitmapFontPath)
                {
                    font = trackerView->currentRowBitmapFont();
                    fontWidth = trackerView->fontWidth();
                }
                else
                {
                    font = trackerView->bitmapFontInstrument();
                    fontWidth = trackerView->fontWidthInstrument();
                }

                trackerView->drawText(instrument, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                numPixels += instrument.length() * fontWidth;
                trackerView->drawText(trackerView->separatorInstrument(), painter, trackerView->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,font);
                numPixels += trackerView->separatorInstrument().length() * fontWidth;

                if (volume == trackerView->emptyVolume() && !trackerView->noEmptyVolumeColor() && j != currentRow)
                {
                    if (!alternateChannelColors)
                    {
                        painter->setPen(trackerView->colorEmpty());
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

                trackerView->drawText(volume, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                numPixels += volume.length() * fontWidth;
                trackerView->drawText(trackerView->separatorVolume(), painter, trackerView->xOffsetRow() + numPixels,
                         yOffset + yPixelPosition,font);
                numPixels += trackerView->separatorVolume().length() * fontWidth;

                if (!trackerView->effectsThenParametersEnabled())
                {
                    if (effect == trackerView->emptyEffect() &&
                        !trackerView->noEmptyEffectColor() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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
                            if (trackerView->useInstrumentColorOnEffectAndParameterFrequency() &&
                                j % trackerView->useInstrumentColorOnEffectAndParameterFrequency() == 0)
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
                            if (trackerView->useInstrumentColorOnEffectAndParameterFrequency() &&
                                j % trackerView->useInstrumentColorOnEffectAndParameterFrequency() == 0)
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

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontEffects().m_bitmapFontPath)
                    {
                        font = trackerView->currentRowBitmapFont();
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font = trackerView->bitmapFontEffects();
                        fontWidth = trackerView->fontWidthEffects();
                    }

                    trackerView->drawText(effect, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                    numPixels += effect.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorEffect(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font);
                    numPixels += trackerView->separatorEffect().length() * fontWidth;

                    if (parameter == trackerView->emptyParameter() &&
                        !trackerView->noEmptyParameterColor() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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
                            if (trackerView->useInstrumentColorOnEffectAndParameterFrequency() &&
                                j % trackerView->useInstrumentColorOnEffectAndParameterFrequency() == 0)
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
                            if (trackerView->useInstrumentColorOnEffectAndParameterFrequency() &&
                                j % trackerView->useInstrumentColorOnEffectAndParameterFrequency() == 0)
                            {
                                painter->setPen(colorInstrumentAlt);
                            }
                            else
                            {
                                painter->setPen(colorParameterAlt);
                            }
                        }
                    }

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath ==
                        trackerView->bitmapFontParameters().m_bitmapFontPath)
                    {
                        font = trackerView->currentRowBitmapFont();
                        trackerView->drawText(parameter, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font = trackerView->bitmapFontParameters();
                        trackerView->drawText(parameter, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                        fontWidth = trackerView->fontWidthParameters();
                    }

                    numPixels += parameter.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorParameter(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font);
                    numPixels += trackerView->separatorParameter().length() * fontWidth;

                    if (effect2 == trackerView->emptyEffect() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontEffects().m_bitmapFontPath)
                    {
                        font = trackerView->currentRowBitmapFont();
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font = trackerView->bitmapFontEffects();
                        fontWidth = trackerView->fontWidthEffects();
                    }

                    trackerView->drawText(effect2, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font);
                    numPixels += effect2.length() * fontWidth, font;
                    trackerView->drawText(trackerView->separatorEffect2(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font);
                    numPixels += trackerView->separatorEffect2().length() * fontWidth;

                    if (parameter2 == trackerView->emptyParameter() &&
                        !trackerView->noEmptyParameter2Color() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath ==
                        trackerView->bitmapFontParameters().m_bitmapFontPath)
                    {
                        font0 = trackerView->currentRowBitmapFont();
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font0 = trackerView->bitmapFontParameters();
                        fontWidth = trackerView->fontWidthParameters();
                    }

                    trackerView->drawText(parameter2, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font0);
                    numPixels += parameter2.length() * fontWidth, font0;
                    trackerView->drawText(trackerView->separatorParameter2(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font0);
                    numPixels += trackerView->separatorParameter2().length() * fontWidth;
                }
                else
                {
                    if (effect == trackerView->emptyEffect() &&
                        !trackerView->noEmptyEffectColor() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontEffects().m_bitmapFontPath)
                    {
                        font00 = trackerView->currentRowBitmapFont();
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font00 = trackerView->bitmapFontEffects();
                        fontWidth = trackerView->fontWidthEffects();
                    }

                    trackerView->drawText(effect, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font00);
                    numPixels += effect.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorEffect(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font00);
                    numPixels += trackerView->separatorEffect().length() * fontWidth;

                    if (effect2 == trackerView->emptyEffect() &&
                        !trackerView->noEmptyEffect2Color() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    trackerView->drawText(effect2, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font00);
                    numPixels += effect2.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorEffect2(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font00);
                    numPixels += trackerView->separatorEffect2().length() * fontWidth;

                    if (parameter == trackerView->emptyParameter() &&
                        !trackerView->noEmptyParameterColor() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    if (j == currentRow &&
                        trackerView->bitmapFont().m_bitmapFontPath ==
                        trackerView->bitmapFontParameters().m_bitmapFontPath)
                    {
                        font4 = trackerView->currentRowBitmapFont();
                        fontWidth = trackerView->fontWidth();
                    }
                    else
                    {
                        font4 = trackerView->bitmapFontParameters();
                        fontWidth = trackerView->fontWidthParameters();
                    }

                    trackerView->drawText(parameter, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font4);
                    numPixels += parameter.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorParameter(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font4);
                    numPixels += trackerView->separatorParameter().length() * fontWidth;

                    if (parameter2 == trackerView->emptyParameter() &&
                        !trackerView->noEmptyParameterColor() && j != currentRow)
                    {
                        if (!alternateChannelColors)
                        {
                            painter->setPen(trackerView->colorEmpty());
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

                    trackerView->drawText(parameter2, painter, trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,font4);
                    numPixels += parameter2.length() * fontWidth;
                    trackerView->drawText(trackerView->separatorParameter2(), painter, trackerView->xOffsetRow() + numPixels,
                             yOffset + yPixelPosition,font4);
                    numPixels += trackerView->separatorParameter2().length() * fontWidth;
                }
            }

            if (trackerView->rowNumbersLastChannelEnabled())
            {
                painter->setPen(trackerView->colorDefault());

                if (j == currentRow &&
                    trackerView->bitmapFont().m_bitmapFontPath == trackerView->bitmapFontRowNumber().m_bitmapFontPath)
                {
                    trackerView->drawText(trackerView->separatorRowNumberLast() + row, painter,
                             trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,trackerView->currentRowBitmapFont());
                    fontWidth = trackerView->fontWidth();
                }
                else
                {
                    trackerView->drawText(trackerView->separatorRowNumberLast() + row, painter,
                             trackerView->xOffsetRow() + numPixels, yOffset + yPixelPosition,trackerView->bitmapFontRowNumber());
                    fontWidth = trackerView->fontWidthRowNumber();
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

            if (trackerView->linesBetweenRows() && j != currentRow && j != currentRow - 1)
            {
                painter->fillRect(trackerView->xOffsetRow(),
                                  yOffset + yPixelPosition + trackerView->yOffsetRowHighlight(),
                                  trackerView->xOffsetRow() + numPixels - trackerView->
                                  highlightBackgroundOffset(), 1, QColor(0, 0, 0));
            }

            j++;

            if (islibopenmpt || isLibxmp || isSunvox)
            {
                i += info->numChannels;
            }
            else if (isHivelytracker)
            {
                i++;
            }
        }

    }

    trackerView->paintAbove(painter, height, currentRow);
}

void Tracker::drawTop(QPainter* painter) const {
    trackerView->paintTop(painter,info,m_currentPattern, m_currentPosition, m_currentSpeed,m_currentBPM, m_currentRow);
}

void Tracker::drawVuMeters(QPainter* painter) const {
    trackerView->drawVuMeters(painter);
}

void Tracker::paint(QPainter* painter, QPaintEvent* event)
{
    if (trackerView==nullptr)
        return;

    const QSize renderSize = painter->viewport().size();
    const bool sizeChanged = renderSize != m_lastBufferSize;

    unsigned int currentPattern = 0;
    unsigned int currentRow = 0;
    unsigned int currentPosition = 0;
    unsigned int currentSpeed = 0;
    unsigned int currentBPM = 0;
    const unsigned int previousOldSample = trackerView->getCurrentSample();
    bool updateSample= false;

    if(m_currentSample != previousOldSample)
    {
        m_currentSample=trackerView->getCurrentSample();
        updateSample=true;
    }

    const auto &sm = SoundManager::getInstance();

    currentPosition = sm.getPosition(FMOD_TIMEUNIT_MODORDER);
    currentRow = sm.getPosition(FMOD_TIMEUNIT_MODROW);
    currentPattern = sm.getPosition(FMOD_TIMEUNIT_MODPATTERN);
    currentSpeed = sm.getPosition(FMOD_TIMEUNIT_SPEED);
    currentBPM = sm.getPosition(FMOD_TIMEUNIT_BPM);

    const bool trackerToUpdate = currentPosition != m_currentPositionBuffer ||
                                 currentRow != m_currentRowBuffer ||
                                 m_currentPattern != m_currentPatternBuffer;


    const bool topToUpdate = currentPosition != m_currentPositionBuffer ||
                       m_currentPattern != m_currentPatternBuffer ||
                       m_currentSpeed != m_currentSpeedBuffer ||
                       m_currentBPM != m_currentBPMBuffer || updateSample;

    if (sizeChanged || trackerToUpdate)
    {
        m_currentPositionBuffer = currentPosition;
        m_currentRowBuffer = currentRow;
        m_currentPatternBuffer = currentPattern;
        m_lastBufferSize = renderSize;

        m_backBuffer = QPixmap(renderSize);
        m_backBuffer.fill(Qt::black);

        QPainter bufferPainter(&m_backBuffer);

        const float scaleX = static_cast<float>(renderSize.width())  / static_cast<float>(trackerView->width());
        const float scaleY = static_cast<float>(renderSize.height()) / static_cast<float>(trackerView->height());
        scale = std::min(scaleX, scaleY);
        m_visibleWidth = renderSize.width() / scale;

        bufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        bufferPainter.setRenderHint(QPainter::Antialiasing, true);
        bufferPainter.scale(scale, scale);
        drawPattern(&bufferPainter, m_visibleWidth, true);
    }

    if (m_lastVuSize != renderSize || trackerView->m_renderVuMeter)
    {
        m_lastVuSize = renderSize;
        m_vuBuffer = QPixmap(renderSize);
        m_vuBuffer.fill(Qt::transparent);

        QPainter vuPainter(&m_vuBuffer);
        vuPainter.setCompositionMode(QPainter::CompositionMode_Source);
        vuPainter.setRenderHint(QPainter::Antialiasing, false);
        vuPainter.scale(scale, scale);
        drawVuMeters(&vuPainter);
    }

    // repaint top bar layer
    if ((renderSize != m_lastTopBarSize || topToUpdate) && trackerView->m_renderTop)
    {
        m_currentPositionBuffer = currentPosition;
        m_currentPatternBuffer = currentPattern;
        m_currentSpeedBuffer = currentSpeed;
        m_currentBPMBuffer = currentBPM;
        m_lastTopBarSize = renderSize;
        m_topBarBuffer = QPixmap(renderSize);
        m_topBarBuffer.fill(Qt::transparent);

        QPainter topPainter(&m_topBarBuffer);
        topPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        topPainter.setRenderHint(QPainter::Antialiasing, true);
        topPainter.scale(scale, scale);
        drawTop(&topPainter);
    }

    std::vector<bool> currentMuteState = trackerView->getChannelMuteMask();
    const bool muteChanged = currentMuteState != m_prevMuteState;

    if (const bool renderSizeChanged = m_lastMuteSize != renderSize;
        muteChanged || renderSizeChanged) {
        m_lastMuteSize = renderSize;
        m_muteBuffer = QPixmap(renderSize);
        m_muteBuffer.fill(Qt::transparent);

        QPainter mutePainter(&m_muteBuffer);
        mutePainter.scale(scale, scale);

        if (std::any_of(currentMuteState.begin(), currentMuteState.end(), [](const bool b) { return b; })) {
            trackerView->updateEnabledChannels(&mutePainter);
        }

        m_prevMuteState = currentMuteState;
    }

    // composite all layers to screen

    painter->drawPixmap(0, 0, m_backBuffer);

    if (trackerView->m_renderVuMeter)
        painter->drawPixmap(0, 0, m_vuBuffer);

    if (trackerView->m_renderTop)
        painter->drawPixmap(0, 0, m_topBarBuffer);

    painter->drawPixmap(0, 0, m_muteBuffer);

}