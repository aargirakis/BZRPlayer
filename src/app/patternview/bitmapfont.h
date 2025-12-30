#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include <QBitmap>
#include "various.h"

class BitmapFont {
    PROVIDE_CLASS_NAME()

public:
    BitmapFont();

    BitmapFont(const QString &);

    void buildCharacterLookup();

    int m_fontWidth;
    int m_fontHeight;
    QString m_bitmapFontCharset;
    QString m_bitmapFontPath;
    QBitmap m_characterMap;
    QMap<QChar, QPoint> m_characterPositions;
    QMap<QChar, int> m_characterWidths;
};

#endif // BITMAPFONT_H
