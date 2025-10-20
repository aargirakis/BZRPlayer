#include "VuMetersEffect.h"
#include "soundmanager.h"
#include <algorithm>

VuMetersEffect::VuMetersEffect()
{

}
void VuMetersEffect::ensurePeakSlots(size_t n)
{
    if (m_peaks.size() != n)
        m_peaks.assign(n, Peak(m_peakHeight));
}

void VuMetersEffect::paint(QPainter* painter, bool stereo)
{
    if (!m_enabled) return;

    painter->save();
    painter->setOpacity(m_opacity);

    int numChannels = SoundManager::getInstance().m_Info1->numChannels;
    if (stereo)
    {
        numChannels = SoundManager::getInstance().m_Info1->numChannelsStream;
    }
    ensurePeakSlots(size_t(numChannels));
    //Maxheight is maximum available pixels that vumeter can use (if peaks are enabled, we have to subtract the peak height later)
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

    int vumeterWidth = (canvasWidth / numChannels) * (m_widthPct / 100.0); //The vumeter width seen by the user
    int vumeterWidthAvailable = (canvasWidth / numChannels); // //The total width available for a vumeter
    int vumeterHighLightWidth = vumeterWidth * 0.1; //10 % of vumeter width
    int vumeterLeftPadding = (vumeterWidthAvailable - vumeterWidth) / 2; //Left padding of vumeter
    QRect rectVU;
    QRect rectVUHiLite;
    QRect rectVUShadow;
    linearGrad.setColorAt(0, m_colTop);
    linearGrad.setColorAt(0.5, m_colMid);
    linearGrad.setColorAt(1, m_colBot);

    for (int unsigned i = 0; i < numChannels; i++)
    {
        //volume is in percentage, 0-100
        unsigned char volume;
        if (stereo)
        {
            volume = SoundManager::getInstance().getSoundData(i);
        }
        else
        {
            volume = static_cast<unsigned char>(SoundManager::getInstance().m_Info1->modVUMeters[i]);
        }

        int vumeterCurrentHeight = (volume * maxHeight / 100);

        int vuWidth = vumeterWidth;
        int xPos = (i * vumeterWidthAvailable) + vumeterLeftPadding;

        if (m_peaksEnabled)
        {
            const int availableBelowPeak = std::max(0, maxHeight - m_peakHeight);
            const int vuHeight = (availableBelowPeak * vumeterCurrentHeight) / maxHeight;
            const int yPos = maxHeight - vuHeight;
            rectVU = QRect(xPos, yPos, vuWidth, vuHeight);
            rectVUHiLite = QRect(xPos, yPos, vumeterHighLightWidth, vuHeight);
            rectVUShadow = QRect(xPos + vuWidth - vumeterHighLightWidth, yPos, vumeterHighLightWidth, vuHeight);
        }
        else
        {
            const int vuHeight = vumeterCurrentHeight;
            const int yPos = maxHeight - vumeterCurrentHeight;
            rectVU = QRect(xPos, yPos, vuWidth, vuHeight);
            rectVUHiLite = QRect(xPos, yPos, vumeterHighLightWidth, vuHeight);
            rectVUShadow = QRect(xPos + vuWidth - vumeterHighLightWidth, yPos, vumeterHighLightWidth, vuHeight);
        }

        painter->fillRect(rectVU, QBrush(linearGrad));
        painter->fillRect(rectVUHiLite, QColor(255, 255, 255, 50));
        painter->fillRect(rectVUShadow, QColor(0, 0, 0, 80));

        //Peaks
        if (m_peaksEnabled)
        {
            if (vumeterCurrentHeight >= m_peaks[i].currentY)
            {
                m_peaks[i].currentY = vumeterCurrentHeight;
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
            QRect peakHiLite(xPos, maxHeight - m_peaks[i].currentY, vumeterHighLightWidth, m_peakHeight);
            QRect peakDark(xPos + vuWidth - vumeterHighLightWidth, maxHeight - m_peaks[i].currentY, vumeterHighLightWidth, m_peakHeight);

            painter->fillRect(peak, m_colPeak);
            painter->fillRect(peakHiLite, QColor(255, 255, 255, 50));
            painter->fillRect(peakDark, QColor(0, 0, 0, 80));
        }

        // Mirror the exact same geometry for reflection
        if (m_reflectionEnabled)
        {
            painter->save();

            const qreal reflectScale = 0.5;
            painter->setClipRect(QRectF(0, maxHeight, canvasWidth, maxHeight), Qt::ReplaceClip);

            painter->setOpacity(m_reflectionOpacity * m_opacity);

            QTransform t;
            t.translate(0, 1.5 * qreal(maxHeight));
            t.scale(1.0, -reflectScale);
            painter->setTransform(t, true);

            // Use floating rects to avoid rounding artifacts under scaling
            QRectF barF(rectVU);
            QRectF barHLF(rectVUHiLite);
            QRectF barDarkF(rectVUShadow);

            painter->fillRect(barF, QBrush(linearGrad));
            painter->fillRect(barHLF, QColor(255, 255, 255, 50));
            painter->fillRect(barDarkF, QColor(0, 0, 0, 80));

            if (m_peaksEnabled)
            {
                QRectF peakF( qreal(xPos), qreal(maxHeight - m_peaks[i].currentY), qreal(vuWidth), qreal(m_peakHeight) );
                QRectF peakHLF( qreal(xPos), qreal(maxHeight - m_peaks[i].currentY), qreal(vumeterHighLightWidth), qreal(m_peakHeight) );
                QRectF peakDarkF( qreal(xPos + vuWidth - vumeterHighLightWidth), qreal(maxHeight - m_peaks[i].currentY), qreal(vumeterHighLightWidth), qreal(m_peakHeight) );
                painter->fillRect(peakF, m_colPeak);
                painter->fillRect(peakHLF, QColor(255, 255, 255, 50));
                painter->fillRect(peakDarkF, QColor(0, 0, 0, 80));
            }
            painter->restore();
        }
    }
    painter->restore();
}

