#include "PrinterEffect.h"
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>

void PrinterEffect::setText(const QString& t) {
    m_text = t.toUpper().trimmed();
    relayoutRows();
}

void PrinterEffect::setFont(const QString& pngPath) {
    if (!loadBitmapFont(pngPath)) return;
    m_fontPngPath = pngPath;
    rebuildAtlas();
    relayoutRows();
}

void PrinterEffect::setFontScaleX(int v) {
    if (v < 1) v = 1;
    m_scaleXInt = v;
    rebuildAtlas();
    relayoutRows();
}

void PrinterEffect::setFontScaleY(int v) {
    if (v < 1) v = 1;
    m_scaleYInt = v;
    rebuildAtlas();
    relayoutRows();
}

void PrinterEffect::paint(QPainter* p) {
    if (!m_enabled) return;
    if (m_rows.isEmpty() || m_atlas.isNull() || m_charset.isEmpty()) return;

    advanceFade();

    int finalY = (m_h / 2) - ((m_rows.size() + 1) * (m_fontHeight + m_verticalRowSpace));
    if (finalY < m_fontHeight) finalY = m_fontHeight;

    const int scrollerYPosition = m_h / 2 - m_fontHeight / 2;
    int reflectionY = (scrollerYPosition + m_verticalScroll + m_bottomY + m_fontHeight / 2)
                      + (m_fontHeight / 2) * m_rows.size();
    reflectionY += m_fontHeight;

    // draw rows
    int pos = 0;
    for (int r = 0; r < m_rows.size(); ++r) {
        const QString& row = m_rows[r];
        int rowWidthPx = row.size() * m_fontWidth;
        int finalX = (m_w / 2) - (rowWidthPx / 2);

        for (int n = 0; n < row.size(); ++n, ++pos) {
            const int g = m_glyphs[pos];
            if (g < 0) continue; // skip missing glyphs

            const int x = finalX + n * m_fontWidth;
            const int y = finalY + r * (m_fontHeight + m_verticalRowSpace);

            p->setOpacity(m_fadeOpacity);
            p->setTransform(QTransform().scale(m_scaleX, m_scaleY));
            p->drawPixmap(x, y, m_fontWidth, m_fontHeight,
                          m_atlas, g * m_fontWidth, 0, m_fontWidth, m_fontHeight);

            // reflection
            if (m_reflectionEnabled) {
                p->setOpacity(m_fadeOpacity * m_reflectionOpacity);
                p->setTransform(QTransform().scale(m_scaleX, -1 * m_scaleY));
                p->drawPixmap(
                        x,
                        (reflectionY + (m_rows.size() - r * (m_fontHeight + m_verticalRowSpace) / 2)) * -1,
                        m_fontWidth, m_fontHeight / 2,
                        m_atlas, g * m_fontWidth, 0, m_fontWidth, m_fontHeight
                );
            }
        }
    }

    // restore defaults for next effects
    p->setOpacity(1.0);
    p->setTransform(QTransform()); // reset
}

bool PrinterEffect::loadBitmapFont(const QString& pngPath) {
    const int dot = pngPath.lastIndexOf('.');
    if (dot == -1) return false;
    QFile inf(pngPath.left(dot) + ".inf");
    if (!inf.open(QIODevice::ReadOnly)) return false;

    QTextStream s(&inf);
    QString line;

    line = s.readLine(); m_fontWOrig = line.toInt();
    line = s.readLine(); m_fontHOrig = line.toInt();
    line = s.readLine(); m_charset   = line;

    return true;
}

void PrinterEffect::rebuildAtlas() {
    if (m_fontPngPath.isEmpty()) return;
    QPixmap raw(m_fontPngPath);
    if (raw.isNull()) return;

    m_atlas = raw.scaled(raw.width() * m_scaleXInt,
                         raw.height() * m_scaleYInt,
                         Qt::IgnoreAspectRatio,
                         Qt::FastTransformation);
    m_fontWidth  = m_fontWOrig * m_scaleXInt;
    m_fontHeight = m_fontHOrig * m_scaleYInt;
}

void PrinterEffect::relayoutRows() {
    // re-wrap based on current canvas width and font width
    m_rows.clear();
    m_glyphs.clear();

    if (m_text.isEmpty() || m_fontWidth <= 0) return;

    const int maxCharsPerRow = qMax(1, m_w / m_fontWidth);

    QString row;
    for (int i = 0; i < m_text.size(); ++i) {
        QChar ch = m_text.at(i);
        row += ch;
        if (ch == ' ' || ch == '-' || ch == '.' || row.size() == maxCharsPerRow) {
            if (!row.trimmed().isEmpty())
                m_rows.append(row);
            row.clear();
        }
    }
    if (!row.trimmed().isEmpty()) m_rows.append(row.trimmed());

    QStringList merged;
    for (int i = 0; i < m_rows.size(); ++i) {
        if (i < m_rows.size()-1 &&
            (m_rows[i].size() + m_rows[i+1].size() <= maxCharsPerRow)) {
            merged.append(QString(m_rows[i] + m_rows[i+1]).trimmed());
            ++i;
        } else {
            merged.append(m_rows[i].trimmed());
        }
    }
    m_rows = merged;

    rebuildGlyphs();
}

void PrinterEffect::rebuildGlyphs() {
    m_glyphs.clear();
    int total = 0;
    for (const auto& r : m_rows) total += r.size();
    m_glyphs.reserve(total);

    for (const auto& r : m_rows) {
        for (int i = 0; i < r.size(); ++i) {
            int idx = m_charset.indexOf(r.at(i));
            m_glyphs.push_back(idx); // -1 if not found -> skip at draw time
        }
    }
}

void PrinterEffect::advanceFade() {
    m_fadeWait--;
    if (m_fadeWait <= 0) {
        m_fadeWait = 0;
        m_fadeOpacity += 0.01 * m_fadeDir;
    }

    if (m_fadeOpacity > 1.0) {
        m_fadeOpacity = 1.0;
        m_fadeWait = 200;
        m_fadeDir = -1;
    } else if (m_fadeOpacity < 0.0) {
        m_fadeOpacity = 0.0;
        m_fadeWait = 100;
        m_fadeDir = 1;
    }
}
