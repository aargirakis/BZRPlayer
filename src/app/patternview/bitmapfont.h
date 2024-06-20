#ifndef BITMAPFONT_H
#define BITMAPFONT_H
#include <QString>
#include <QBitmap>
#include <QMap>
#include <QPoint>
class BitmapFont
{
public:
    BitmapFont();
    BitmapFont(QString);
    void buildCharacterLookup();

    int m_fontWidth;
    int m_fontHeight;
    QString m_bitmapFontCharset;
    QString m_bitmapFontPath;
    QBitmap m_characterMap;
    QMap<QChar, QPoint> m_characterPositions;
    QMap<QChar, int> m_characterWidths;

private:

};



#endif // BITMAPFONT_H
