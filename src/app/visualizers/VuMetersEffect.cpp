#include "soundmanager.h"
#include "VuMetersEffect.h"

VuMetersEffect::VuMetersEffect()
{

}

void VuMetersEffect::ensurePeakSlots(const size_t n)
{
    if (m_peaks.size() != n)
        m_peaks.assign(n, Peak(m_peakHeight));
}

void VuMetersEffect::paint(QPainter* painter, bool stereo)
{
    if (!m_enabled) return;

    painter->save();
    painter->setOpacity(m_opacity);

    auto &sm = SoundManager::getInstance();
    const auto &info = sm.info;

    int numChannels = info->numChannels;

    if (stereo)
    {
        numChannels = info->numChannelsStream;
    }

    ensurePeakSlots(static_cast<size_t>(numChannels));
    // maxheight is maximum available pixels that vu-meter can use (if peaks are enabled, we have to subtract the peak height later)
    int maxHeight = m_maxHeightOverride;

    QLinearGradient linearGrad;

    if (m_peaksEnabled)
    {
        linearGrad = QLinearGradient(QPointF(0, m_peakHeight), QPointF(0, maxHeight));
    }
    else
    {
        linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, maxHeight));
    }

    int vuMeterWidth = canvasWidth / numChannels * (m_widthPct / 100.0); // the vu-meter width seen by the user
    int vuMeterWidthAvailable = canvasWidth / numChannels; // the total width available for a vu-meter
    int vuMeterHighLightWidth = vuMeterWidth * 0.1; // 10 % of vu-meter width
    int vuMeterLeftPadding = (vuMeterWidthAvailable - vuMeterWidth) / 2; // left padding of vu-meter
    QRect rectVU;
    QRect rectVUHiLite;
    QRect rectVUShadow;
    linearGrad.setColorAt(0, m_colTop);
    linearGrad.setColorAt(0.5, m_colMid);
    linearGrad.setColorAt(1, m_colBot);

    for (int unsigned i = 0; i < numChannels; i++)
    {
        // volume is in percentage, 0-100
        unsigned char volume;

        if (stereo)
        {
            volume = static_cast<unsigned char>(sm.getSoundData(i));
        }
        else
        {
            volume = info->modVuMeters[i];
        }

        int vuMeterCurrentHeight = volume * maxHeight / 100;

        int vuWidth = vuMeterWidth;
        int xPos = i * vuMeterWidthAvailable + vuMeterLeftPadding;

        if (m_peaksEnabled)
        {
            const int availableBelowPeak = std::max(0, maxHeight - m_peakHeight);
            const int vuHeight = availableBelowPeak * vuMeterCurrentHeight / maxHeight;
            const int yPos = maxHeight - vuHeight;
            rectVU = QRect(xPos, yPos, vuWidth, vuHeight);
            rectVUHiLite = QRect(xPos, yPos, vuMeterHighLightWidth, vuHeight);
            rectVUShadow = QRect(xPos + vuWidth - vuMeterHighLightWidth, yPos, vuMeterHighLightWidth, vuHeight);
        }
        else
        {
            const int vuHeight = vuMeterCurrentHeight;
            const int yPos = maxHeight - vuMeterCurrentHeight;
            rectVU = QRect(xPos, yPos, vuWidth, vuHeight);
            rectVUHiLite = QRect(xPos, yPos, vuMeterHighLightWidth, vuHeight);
            rectVUShadow = QRect(xPos + vuWidth - vuMeterHighLightWidth, yPos, vuMeterHighLightWidth, vuHeight);
        }

        painter->fillRect(rectVU, QBrush(linearGrad));
        painter->fillRect(rectVUHiLite, QColor(255, 255, 255, 50));
        painter->fillRect(rectVUShadow, QColor(0, 0, 0, 80));

        // peaks
        if (m_peaksEnabled)
        {
            if (vuMeterCurrentHeight >= m_peaks[i].currentY)
            {
                m_peaks[i].currentY = vuMeterCurrentHeight;
                m_peaks[i].timeLeftStill = 10;
                m_peaks[i].currentSpeedDrop = 1;
            }
            else
            {
                if (m_peaks[i].timeLeftStill <= 0)
                {
                    m_peaks[i].currentSpeedDrop++;
                    m_peaks[i].currentY -= m_peaks[i].currentSpeedDrop;

                    if(m_peaks[i].currentY < m_peakHeight)
                    {
                        m_peaks[i].currentY=m_peakHeight;
                    }
                }
                else
                {
                    m_peaks[i].timeLeftStill -= 1;
                }
            }

            QRect peak(xPos, maxHeight - m_peaks[i].currentY, vuWidth, m_peakHeight);
            QRect peakHiLite(xPos, maxHeight - m_peaks[i].currentY, vuMeterHighLightWidth, m_peakHeight);
            QRect peakDark(xPos + vuWidth - vuMeterHighLightWidth, maxHeight - m_peaks[i].currentY, vuMeterHighLightWidth, m_peakHeight);

            painter->fillRect(peak, m_colPeak);
            painter->fillRect(peakHiLite, QColor(255, 255, 255, 50));
            painter->fillRect(peakDark, QColor(0, 0, 0, 80));
        }

        // mirror the exact same geometry for reflection
        if (m_reflectionEnabled)
        {
            painter->save();

            constexpr qreal reflectScale = 0.5;
            painter->setClipRect(QRectF(0, maxHeight, canvasWidth, maxHeight), Qt::ReplaceClip);

            painter->setOpacity(m_reflectionOpacity * m_opacity);

            QTransform t;
            t.translate(0, 1.5 * static_cast<qreal>(maxHeight));
            t.scale(1.0, -reflectScale);
            painter->setTransform(t, true);

            // use floating rects to avoid rounding artifacts under scaling
            QRectF barF(rectVU);
            QRectF barHLF(rectVUHiLite);
            QRectF barDarkF(rectVUShadow);

            painter->fillRect(barF, QBrush(linearGrad));
            painter->fillRect(barHLF, QColor(255, 255, 255, 50));
            painter->fillRect(barDarkF, QColor(0, 0, 0, 80));

            if (m_peaksEnabled)
            {
                QRectF peakF(xPos, maxHeight - m_peaks[i].currentY, vuWidth, m_peakHeight);
                QRectF peakHLF(xPos, maxHeight - m_peaks[i].currentY, vuMeterHighLightWidth, m_peakHeight);
                QRectF peakDarkF(xPos + vuWidth - vuMeterHighLightWidth, maxHeight - m_peaks[i].currentY,
                                 vuMeterHighLightWidth, m_peakHeight);
                painter->fillRect(peakF, m_colPeak);
                painter->fillRect(peakHLF, QColor(255, 255, 255, 50));
                painter->fillRect(peakDarkF, QColor(0, 0, 0, 80));
            }

            painter->restore();
        }
    }

    painter->restore();
}

