#ifndef BZR2_PRINTEREFFECT_H
#define BZR2_PRINTEREFFECT_H

#include <QPainter>

class PrinterEffect {
public:
    PrinterEffect() = default;

    void setCanvasSize(const int w, const int h) { m_w = w; m_h = h; relayoutRows(); }
    void setText(const QString& t);
    void setScale(const qreal sx, const qreal sy) { m_scaleX = sx; m_scaleY = sy; }

    // settings
    void setEnabled(const bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }
    void setFont(const QString& pngPath);
    QString getFont() const { return m_fontPngPath; }
    void setFontScaleX(int v);
    void setFontScaleY(int v);
    int getFontScaleX() const {return m_scaleXInt;}
    int getFontScaleY() const {return m_scaleYInt;}
    // end settings

    void setVerticalScrollPosition(const int v) { m_verticalScroll = v; }
    void setBottomYFromScroller(const int v) { m_bottomY = v; }

    // reflection
    void setReflectionEnabled(const bool on) { m_reflectionEnabled = on; }
    void setReflectionOpacity(const double o01) { m_reflectionOpacity = o01; }
    void setReflectionColor(const QColor& c) { m_reflectionColor = c; }

    void paint(QPainter* p);

private:
    // settings
    bool m_enabled = false;
    QString m_fontPngPath;
    int m_scaleXInt = 1;
    int m_scaleYInt = 1;
    // end settings

    int m_w = 320, m_h = 256;
    qreal m_scaleX = 1.0, m_scaleY = 1.0;

    // font/atlas
    QString m_charset;
    QPixmap m_atlas;
    int m_fontWOrig = 8, m_fontHOrig = 8;
    int m_fontWidth = 8, m_fontHeight = 8;

    // text
    QString m_text;
    QStringList m_rows;
    QVector<int> m_glyphs;
    int m_verticalRowSpace = 2;

    // fade animation
    double m_fadeOpacity = 0.0;
    int m_fadeDir = 1;
    int m_fadeWait = 100;

    // reflection
    bool m_reflectionEnabled = true;
    double m_reflectionOpacity = 0.04;
    QColor m_reflectionColor = QColor(0,3,46);

    // coupling with scroller for baseline
    int m_verticalScroll = 0;
    int m_bottomY = 0;

    void relayoutRows();
    void rebuildAtlas();
    void rebuildGlyphs();
    void advanceFade();
    bool loadBitmapFont(const QString& pngPath);
};

#endif //BZR2_PRINTEREFFECT_H
