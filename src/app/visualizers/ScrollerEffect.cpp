#include "ScrollerEffect.h"
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <cmath>

bool ScrollerEffect::setFont(const QString& pngPath) {
    if (!loadBitmapFont(pngPath)) return false;
    m_fontPathPng = pngPath;
    rebuildAtlas();
    reinitForCanvas();
    recomputeBottomYMax();
    return true;
}

void ScrollerEffect::setFontScaleX(int v) {
    if (v < 1) v = 1;
    m_scaleXInt = v;
    rebuildAtlas();
    reinitForCanvas();
}

void ScrollerEffect::setFontScaleY(int v) {
    if (v < 1) v = 1;
    m_scaleYInt = v;
    rebuildAtlas();
    reinitForCanvas();
    recomputeBottomYMax();
}

void ScrollerEffect::setText(const QString& t) {
    m_srcText = t;

    const int pixelsPerChar = (m_fontWidth > 0 ? m_fontWidth : 1);
    const int spacesNeeded  = (m_w / pixelsPerChar) + 3;

    QString spaces(spacesNeeded, QChar(' '));
    const QString normalized = normalizeText(t.toUpper().trimmed());

    if (normalized.isEmpty())  m_workText = spaces;
    else                       m_workText = spaces + normalized;

    reinitForCanvas();
}

void ScrollerEffect::reinitForCanvas() {
    if (m_fontWidth <= 0) return;

    const int spacesNeeded = (m_w / m_fontWidth) + 3;
    m_letters  = spacesNeeded;
    m_position = m_letters;
    m_sineAngle = 0.0;

    rebuildSlots();
    recomputeBottomYMax(); // initial estimate
}

void ScrollerEffect::rebuildAtlas() {
    if (m_fontPathPng.isEmpty()) return;
    QPixmap raw(m_fontPathPng);
    if (raw.isNull()) return;

    m_atlas = raw.scaled(raw.width()  * m_scaleXInt,
                         raw.height() * m_scaleYInt,
                         Qt::IgnoreAspectRatio,
                         Qt::FastTransformation);
    m_fontWidth  = m_fontWOrig * m_scaleXInt;
    m_fontHeight = m_fontHOrig * m_scaleYInt;
}

void ScrollerEffect::rebuildSlots() {
    m_chars.assign(m_letters, 0);
    m_x.assign(m_letters, 0);

    // pre-fill the visible window with the initial characters of m_workText
    for (int n = 0; n < m_letters; ++n) {
        const QChar ch = (n < m_workText.size() ? m_workText.at(n) : QChar(' '));
        m_chars[n] = m_charset.indexOf(ch);
        m_x[n]     = n * m_fontWidth;
    }
}

QString ScrollerEffect::normalizeText(QString text) const {
    QString a1 = "ÀÁÂÃÄÅĀĂáàāãåä"; QString a2 = "A";
    QString b1 = "ß";              QString b2 = "B";
    QString c1 = "Çç";             QString c2 = "C";
    QString e1 = "ËÉÈÊèéêë&";      QString e2 = "E";
    QString f1 = "ƒ";              QString f2 = "F";
    QString i1 = "ÍÌÎÏìíîï";       QString i2 = "I";
    QString n1 = "Ññ";             QString n2 = "N";
    QString o1 = "ØÓÒÔÕÖòóôðøõöø"; QString o2 = "O";
    QString u1 = "ÛÙÚÜùúüûµ";      QString u2 = "U";
    QString x1 = "×";              QString x2 = "X";
    QString y1 = "Ýýÿ";            QString y2 = "Y";
    QString q1 = "¿";              QString q2 = "?";
    QString e3 = "¡";              QString e4 = "!";
    QString h1 = "¯─~·";           QString h2 = "-";
    QString eq1 = "═‗≈";           QString eq2 = "=";
    QString qt1 = "`´‘’“”";        QString qt2 = "\"";
    QString one1="¹", one2="1"; QString two1="²", two2="2"; QString three1="³", three2="3";
    QString half1="½", half2="1/2"; QString quarter1="¼", quarter2="1/4"; QString threeq1="¾", threeq2="3/4";
    QString arrow1="→", arrow2="->";

    if (text.contains(QRegularExpression("[" + QRegularExpression::escape(
            "$/:" + a1 + b1 + c1 + e1 + f1 + i1 + n1 + o1 + u1 + x1 + y1 + q1 + e3 + h1 + eq1 + qt1
            + one1 + two1 + three1 + arrow1 + half1 + quarter1 + threeq1) + "]")))
    {
        text.replace(QRegularExpression("[ëêéè]"), "e");
    }
    text.replace(QRegularExpression("[" + a1 + "]"), a2);
    text.replace(QRegularExpression("[" + b1 + "]"), b2);
    text.replace(QRegularExpression("[" + c1 + "]"), c2);
    text.replace(QRegularExpression("[" + e1 + "]"), e2);
    text.replace(QRegularExpression("[" + f1 + "]"), f2);
    text.replace(QRegularExpression("[" + i1 + "]"), i2);
    text.replace(QRegularExpression("[" + n1 + "]"), n2);
    text.replace(QRegularExpression("[" + o1 + "]"), o2);
    text.replace(QRegularExpression("[" + u1 + "]"), u2);
    text.replace(QRegularExpression("[" + x1 + "]"), x2);
    text.replace(QRegularExpression("[" + y1 + "]"), y2);
    text.replace(QRegularExpression("[" + q1 + "]"), q2);
    text.replace(QRegularExpression("[" + e3 + "]"), e4);
    text.replace(QRegularExpression("[" + h1 + "]"), h2);
    text.replace(QRegularExpression("[" + eq1 + "]"), eq2);
    text.replace(QRegularExpression("[" + qt1 + "]"), qt2);
    text.replace(QRegularExpression("[" + one1 + "]"), one2);
    text.replace(QRegularExpression("[" + two1 + "]"), two2);
    text.replace(QRegularExpression("[" + three1 + "]"), three2);
    text.replace(QRegularExpression("[" + half1 + "]"), half2);
    text.replace(QRegularExpression("[" + quarter1 + "]"), quarter2);
    text.replace(QRegularExpression("[" + threeq1 + "]"), threeq2);
    text.replace(QRegularExpression("[" + arrow1 + "]"), arrow2);

    return text;
}

bool ScrollerEffect::loadBitmapFont(const QString& pngPath) {
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
auto overlaps1D = [](int a0, int a1, int b0, int b1) {
    return a0 <= b1 && b0 <= a1;
};
void ScrollerEffect::paint(QPainter* painter, bool paused) {
    if (!m_enabled) return;
    if (m_atlas.isNull() || m_charset.isEmpty() || m_letters <= 0) return;
    if (m_fontWidth <= 0 || m_fontHeight <= 0) return;


    for (int n = 0; n < m_letters; ++n) {
        // per-pixel column blit)
        for (int xC = 0; xC < m_fontWidth; xC += m_scaleXInt) {
            int y;
            if (!m_sinusFontScaling) {
                y = int(std::sin(((m_x[n] + xC) * m_sinusFreq) + m_sineAngle) * m_amplitude) * m_scaleYInt;
            } else {
                y = int(std::sin(((m_x[n] + xC) * m_sinusFreq) + m_sineAngle) * m_amplitude);
                y = y * m_scaleYInt;
            }
            const int colW = m_scaleXInt;           // one column width (logical)
            const int xPix = m_x[n] + xC;

            const int tol = 1;                      // 1px padding on both sides
            const bool xVisible = overlaps1D(xPix, xPix + colW - 1, -tol, m_w - 1 + tol);

            const int scrollerYPosition = m_h / 2 - m_fontHeight / 2;
            const int finalY = y + scrollerYPosition + m_verticalScroll;

            const bool baseVisible = xVisible &&
                                     overlaps1D(finalY, finalY + m_fontHeight - 1,
                                                0, m_h - 1);
            if (baseVisible) {
                painter->setOpacity(1.0);
                painter->setTransform(QTransform().scale(m_scaleX, m_scaleY));
                const int glyph = m_chars[n];
                if (glyph >= 0) {
                    painter->drawPixmap(xPix, finalY,
                                        colW, m_fontHeight,
                                        m_atlas, glyph * m_fontWidth + xC, 0,
                                        colW, m_fontHeight);
                }
            }

            // reflection uses its own Y
            if (m_reflectionEnabled) {
                const int glyph = m_chars[n];
                if (glyph >= 0) {
                    const int refDrawY =
                            y - scrollerYPosition - m_verticalScroll
                            - (m_bottomYMax * 2)
                            - m_fontHeight
                            - (m_fontHeight / 2);

                    //Draw with a flipped Y transform, cull using the unflipped top/bottom
                    const bool refVisible = xVisible &&
                                            overlaps1D(refDrawY, refDrawY + (m_fontHeight/2) - 1,
                                                       -(m_h - 1), 0); // safe, but you can simply skip Y-cull for reflection if you want

                    if (refVisible) {
                        painter->setOpacity(m_reflectionOpacity);
                        painter->setTransform(QTransform().scale(m_scaleX, -1 * m_scaleY));
                        painter->drawPixmap(xPix, refDrawY,
                                            colW, m_fontHeight / 2,
                                            m_atlas, glyph * m_fontWidth + xC, 0,
                                            colW, m_fontHeight);
                    }
                }
            }
        }

        // advance scroll (unless paused)
        if (!paused) m_x[n] -= m_scrollSpeed;

        // recycle letters
        if (m_x[n] < -m_fontWidth) {
            m_x[n] = (m_letters - 1) * m_fontWidth;
            // next glyph from stream
            if (!m_workText.isEmpty()) {
                const QChar ch = m_workText.at(m_position % m_workText.size());
                m_chars[n] = m_charset.indexOf(ch);
                ++m_position;
                if (m_position >= m_workText.size()) m_position = 0;
            }
        }
    }

    m_sineAngle += m_sinusSpeed;

    // restore defaults for other effects
    painter->setOpacity(1.0);
    painter->setTransform(QTransform());
}
