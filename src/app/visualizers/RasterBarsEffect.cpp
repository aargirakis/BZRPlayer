#include "RasterBarsEffect.h"

#include <QRandomGenerator>
#include <QtMath>
#include <QColor>

RasterBarsEffect::RasterBarsEffect() {
    rebuildColors();
}

void RasterBarsEffect::setOpacity(int percent) {
    if (percent < 0)  percent = 0;
    if (percent > 100) percent = 100;
    m_opacity = percent / 100.0;
}

void RasterBarsEffect::setCount(int n) {
    if (n < 1) n = 1;
    if (m_count == n) return;
    m_count = n;
    rebuildColors();
}

void RasterBarsEffect::setBarHeight(int px) {
    if (px < 2) px = 2;
    if (m_barHeight == px) return;
    m_barHeight = px;
    rebuildColors();
}

void RasterBarsEffect::setVerticalSpacing(int amount) {
    m_verticalSpacing = 33 - amount;
    if (m_verticalSpacing < 1) m_verticalSpacing = 1;
}

void RasterBarsEffect::rebuildColors() {
    m_barBrushes.clear();
    m_barBrushes.reserve(m_count);

    auto hsv = [](double h, double s, double v) {
        QColor c; c.setHsvF(h, s, v); return c;
    };

    for (int n = 0; n < m_count; ++n) {
        const double hue = QRandomGenerator::global()->generateDouble();  // [0..1)
        const double sat = QRandomGenerator::global()->generateDouble();  // [0..1)

        QLinearGradient g(QPointF(0, 0), QPointF(0, 1)); // vertical
        g.setCoordinateMode(QGradient::ObjectBoundingMode); // scale with rect
        g.setColorAt(0.0, hsv(hue, sat, 0.0));
        g.setColorAt(0.5, hsv(hue, sat, 1.0));
        g.setColorAt(1.0, hsv(hue, sat, 0.0));

        m_barBrushes.emplace_back(g);
    }
}

void RasterBarsEffect::paint(QPainter* painter) {
    if (!m_enabled || m_width <= 0 || m_height <= 0) return;

    m_counter += 0.001 * m_speed;
    m_sineAngle += 0.004;

    painter->save();
    painter->setOpacity(m_opacity);

    for (int i = 0; i < m_count; ++i) {
        drawOneBar(painter, i);
    }

    painter->restore();
}

void RasterBarsEffect::drawOneBar(QPainter* painter, int offset) {
    const double amp   = (m_height / 2.0) - (m_barHeight / 2.0);
    const double phase = m_counter
                         + offset / double(m_count * m_verticalSpacing * 0.05)
                         + m_sineAngle;

    double y = ((m_height / 2.0) - m_count) + (qSin(phase) * amp);
    y += m_count - (m_barHeight / 2.0);

    const int iy = int(y);

    const QBrush& brush = m_barBrushes[
            qBound(0, offset, int(m_barBrushes.size()) - 1)
    ];

    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawRect(0, iy, m_width, m_barHeight);
}

