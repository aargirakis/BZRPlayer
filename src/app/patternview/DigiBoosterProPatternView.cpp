#include "DigiBoosterProPatternView.h"

#include <mainwindow.h>
#include <QApplication>
#include <QDir>

DigiBoosterProPatternView::DigiBoosterProPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    octaveOffset = 24;
    m_font = QFont("DigiBoosterPro2");
    m_font.setPixelSize(7);
    m_fontWidth = 6;
    m_font2 = QFont("DigiBoosterPro2 Big");
    m_font2.setPixelSize(7);

    m_bitmapFont = BitmapFont("DigiBoosterPro2");
    m_bitmapFont2 = BitmapFont("DigiBoosterPro2 Big");
    m_fontWidth = 6;
    m_fontHeight = 7;


    rowNumberOffset = 0;

    //m_ColorWindowBackground ="313042";
    m_ColorRowNumberBackground = m_ColorWindowBackground;
    m_colorCurrentRowBackground = QColor(156, 154, 156);
    m_colorCurrentRowForeground = QColor(255, 255, 255);
    m_SeparatorNote = "'";
    m_SeparatorInstrument = "'";
    m_SeparatorParameter = "'";
    effect2Enabled = true;
    parameter2Enabled = true;
    m_linesBetweenRows = true;

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(173, 170, 189);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_xOffsetRow = 8;
    m_yOffsetCurrentRowBefore = -3;
    m_yOffsetCurrentRowAfter = 3;
    m_RowLength = 5 + (15 * m_channels) + 3;

    m_xChannelStart = 30;
    m_channelWidth = 87;
    m_channelxSpace = 3;

    m_colorBackground = QColor(49, 48, 66, 180);
    m_topHeight = 1;
    m_bottomFrameHeight = 2;
}

QFont DigiBoosterProPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont DigiBoosterProPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

DigiBoosterProPatternView::~DigiBoosterProPatternView()
{
}

void DigiBoosterProPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(156, 154, 173);
    QColor colorHilite(239, 255, 255);
    QColor colorShadow(99, 101, 115);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);

    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
        "trackerview" + QDir::separator() + "digiboosterpro2_top.png";
    QImage spriteSheet(imagepath);

    //scrollbar

    QRectF sourceScrollBarBg(20, 4, 17, 2);

    int numberOfScrollBarBgPieces = ((height - 4) / (2)) + 1;

    for (int i = 0; i < numberOfScrollBarBgPieces; i++)
    {
        QRectF targetLeftBar((29) + m_channels * 90, (2) + i * 2, 17, 2);
        painter->drawImage(targetLeftBar, spriteSheet, sourceScrollBarBg);
    }

    painter->fillRect((26) + m_channels * 90, 0, 2, height - (1), QColor(0, 0, 0));
    painter->fillRect((28) + m_channels * 90, 0, 1, height - (1), QColor(255, 255, 255));
    painter->fillRect((46) + m_channels * 90, 2, 1, height - (4), QColor(255, 255, 255));
    painter->fillRect((47) + m_channels * 90, 2, 1, height - (4), QColor(0, 0, 0));
    painter->fillRect((29) + m_channels * 90, 0, 18, 1, QColor(255, 255, 255));


    float currentRowPos = currentRow / 64.0;
    int yPos = (currentRowPos * (height - 20)) + 1;
    QRectF scrollBarSource(5, 4, 16, 20);
    QRectF scrollBarTarget((30) + m_channels * 90, yPos, 16, 20);
    painter->drawImage(scrollBarTarget, spriteSheet, scrollBarSource);


    //left border
    QRectF sourceLeftBar(0, 4, 5, 128);

    int numberOfLeftBarPieces = ((height - 3) / (128)) + 1;


    for (int i = 0; i < numberOfLeftBarPieces; i++)
    {
        QRectF targetLeftBar(1, (1) + i * 128, 5, 128);
        painter->drawImage(targetLeftBar, spriteSheet, sourceLeftBar);
    }


    painter->fillRect(0, 0, 1, height - 2, QColor(255, 255, 255));
    painter->fillRect(1, 0, 5, 1, QColor(255, 255, 255));
    painter->fillRect((30) + m_channels * 90, height - (3), 17, 1, QColor(255, 255, 255));
    painter->fillRect((30) + m_channels * 90, height - (4), 16, 1, QColor(0, 0, 0));

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //bottom black border
        painter->fillRect((30) + (chan * 90), height - 2, 87, 1, QColor(0, 0, 0));

        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((28 + chan * 90), 0, (28 + chan * 90), (height / 2) - 11);
        painter->drawLine((28 + chan * 90), (height / 2) + 4, (28 + chan * 90), (height - 1));
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((29 + chan * 90), 0, (29 + chan * 90), (height / 2) - 11);
        painter->drawLine((29 + chan * 90), (height / 2) + 4, (29 + chan * 90), (height - 1));
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((30 + chan * 90), 0, (30 + chan * 90), (height / 2) - 11);
        painter->drawLine((30 + chan * 90), (height / 2) + 4, (30 + chan * 90), (height - 1));
    }
    painter->fillRect(0, height - 2, 27, 1, QColor(0, 0, 0));
    painter->fillRect((27) + (m_channels * 90), height - 2, 21, 1, QColor(0, 0, 0));


    //bottom white border
    painter->fillRect(0, height - 1, (47) + m_channels * 90, 1, QColor(255, 255, 255));

    //top black border
    painter->fillRect((6), 0, (21) + m_channels * 90, 1, QColor(0, 0, 0));
}

void DigiBoosterProPatternView::paintBelow(QPainter* painter, int height, __attribute__((unused)) int currentRow)
{
    QColor colorBase(156, 154, 173);
    QColor colorHilite(239, 255, 255);
    QColor colorShadow(99, 101, 115);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);

    //background
    QColor colorBg = QColor(49, 48, 66);
    painter->fillRect(0, 0, (26) + m_channels * 90, height, colorBg);

    //left black frame
    painter->fillRect(6, 0, 2, height, QColor(0, 0, 0));

    //main
    painter->fillRect(8, (height / 2) - 7, (18) + m_channels * 90, 8, QColor(156, 154, 156));
    painter->fillRect(8, (height / 2) + 3, (18) + m_channels * 90, 1, QColor(0, 0, 0));

    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
        "trackerview" + QDir::separator() + "digiboosterpro2_top.png";
    QImage spriteSheet(imagepath);
    QRectF sourceTop(0, 0, 128, 2);
    QRectF sourceBottom(0, 2, 128, 2);

    int numberOfTopPieces = (m_channels * 90 / (128)) + 1;
    for (int i = 0; i < numberOfTopPieces; i++)
    {
        QRectF targetTop(8 + i * 128, (height / 2) - 9, 128, 2);
        QRectF targetBottom(8 + i * 128, (height / 2) + 1, 128, 2);
        painter->drawImage(targetTop, spriteSheet, sourceTop);
        painter->drawImage(targetBottom, spriteSheet, sourceBottom);
    }


    painter->fillRect((m_channels * 90) + 48, (height / 2) - 9, 100, 14, QColor(0, 0, 0));

    //left hilite
    painter->fillRect(7, (height / 2) - 9, 1, 12, QColor(255, 255, 255));
    //top hilite
    painter->fillRect(7, (height / 2) - 10, (19) + m_channels * 90, 1, QColor(255, 255, 255));
}

QString DigiBoosterProPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber + rowNumberOffset, base).toUpper();
    rowNumberStr = rowNumberStr.length() == 1 ? "00" + rowNumberStr : rowNumberStr;
    rowNumberStr = rowNumberStr.length() == 2 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}

QString DigiBoosterProPatternView::effect2(BaseRow* row)
{
    return QString::number(row->effect2, 36).toUpper();
}

QString DigiBoosterProPatternView::effect(BaseRow* row)
{
    return QString::number(row->effect, 36).toUpper();
}
