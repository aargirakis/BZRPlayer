#include <QDir>
#include "bitmapfont.h"
#include "mainwindow.h"

BitmapFont::BitmapFont() {
}

BitmapFont::BitmapFont(const QString &fontName) {
    const QString fontFilePathWithoutExt = dataPath + RESOURCES_DIR + "/trackerview/fonts/" + fontName;
    const QString fontFilePath = fontFilePathWithoutExt + ".inf";

    QFile myFile(fontFilePath);

    if (!myFile.open(QIODevice::ReadOnly)) {
        logErrorQ("Couldn't load font file " + fontFilePath, getClassName());
        return /*false*/;
    }

    bool variableWidth = false;
    QTextStream stream(&myFile);
    QString line = stream.readLine();
    m_fontWidth = line.toInt();

    if (m_fontWidth == 0) {
        // we have a variable width font
        variableWidth = true;
    }

    line = stream.readLine();
    m_fontHeight = line.toInt();
    line = stream.readLine();
    m_bitmapFontCharset = line;

    if (variableWidth) {
        int fontWidth = 0;
        int fontPositionX = 0;

        for (int i = 0; i < m_bitmapFontCharset.length(); i++) {
            fontWidth = stream.readLine().toInt();
            m_characterWidths[m_bitmapFontCharset[i]] = fontWidth;
            m_characterPositions[m_bitmapFontCharset[i]] = QPoint(fontPositionX, 0);
            fontPositionX += fontWidth;
        }
    }

    m_bitmapFontPath = fontFilePathWithoutExt + ".png";
    buildCharacterLookup();
    //return true;
}

void BitmapFont::buildCharacterLookup() {
    m_characterMap = QBitmap(m_bitmapFontPath);

    if (m_characterMap.isNull()) {
        logErrorQ("Couldn't load bitmap font file " + m_bitmapFontPath, getClassName());
        return;
    }

    if (m_characterWidths.isEmpty()) {
        for (int i = 0; i < m_bitmapFontCharset.length(); i++) {
            const int fontX = i * m_fontWidth % m_characterMap.width();
            const int fontY = i * m_fontWidth / m_characterMap.width() * m_fontHeight;
            m_characterPositions[m_bitmapFontCharset[i]] = QPoint(fontX, fontY);
        }

        //        QMapIterator<QChar, QPoint> it(m_characterPositions);
        //        while (it.hasNext()) {
        //            it.next();
        //            std::cout << it.key().toStdString().c_str() << ": " << it.value().x() << "," << it.value().y() << "\n";
        //        }
    }
}
