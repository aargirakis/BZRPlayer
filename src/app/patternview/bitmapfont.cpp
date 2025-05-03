#include "bitmapfont.h"
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <mainwindow.h>

BitmapFont::BitmapFont()
{
}

BitmapFont::BitmapFont(QString filename)
{
    QString fontDir = dataPath + RESOURCES_DIR + "/trackerview/fonts" + QDir::separator();

    QFile myFile(fontDir + filename + ".inf");
    if (myFile.open(QIODevice::ReadOnly))
    {
        bool variableWidth = false;
        QTextStream stream(&myFile);
        QString line = stream.readLine();
        m_fontWidth = line.toInt();
        if (m_fontWidth == 0)
        {
            //we have a variable width font
            variableWidth = true;
        }

        line = stream.readLine();
        m_fontHeight = line.toInt();
        line = stream.readLine();
        m_bitmapFontCharset = line;
        if (variableWidth)
        {
            int fontWidth = 0;
            int fontPositionX = 0;
            for (int i = 0; i < m_bitmapFontCharset.length(); i++)
            {
                fontWidth = stream.readLine().toInt();
                m_characterWidths[m_bitmapFontCharset[i]] = fontWidth;
                m_characterPositions[m_bitmapFontCharset[i]] = QPoint(fontPositionX, 0);
                fontPositionX += fontWidth;
            }
        }
        m_bitmapFontPath = fontDir + filename + ".png";
        //CLogFile::getInstance()->Print(LOGINFO,"Loaded bitmap font:  '%s'",QString(fontDir + filename).toLocal8Bit().constData());
        buildCharacterLookup();
        //return true;
        qDebug() << "loaded font!";
    }
    else
    {
        qDebug() << "Coul not load font!";
        //CLogFile::getInstance()->Print(LOGERROR,"Bitmap font could not be loaded:  '%s'!",QString(fontDir + filename).toLocal8Bit().constData());
        //return false;
    }
}

void BitmapFont::buildCharacterLookup()
{
    m_characterMap = QBitmap(m_bitmapFontPath);

    if (!m_characterMap.isNull() && m_characterWidths.isEmpty())
    {
        int fontX;
        int fontY;
        for (int i = 0; i < m_bitmapFontCharset.length(); i++)
        {
            fontX = (i * m_fontWidth) % m_characterMap.width();
            fontY = (i * m_fontWidth) / m_characterMap.width() * m_fontHeight;
            m_characterPositions[m_bitmapFontCharset[i]] = QPoint(fontX, fontY);
        }
        //        QMapIterator<QChar, QPoint> it(m_characterPositions);
        //        while (it.hasNext()) {
        //            it.next();
        //            std::cout << it.key().toStdString().c_str() << ": " << it.value().x() << "," << it.value().y() << "\n";
        //            flush(std::cout);
        //        }
    }
    else
    {
        //CLogFile::getInstance()->Print(LOGERROR,"Couldn't open bitmap fontfile: %s",m_bitmapFontPath.toStdString().c_str());
    }
}
