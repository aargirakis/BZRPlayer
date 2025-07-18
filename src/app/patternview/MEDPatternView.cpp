#include "MEDPatternView.h"

MEDPatternView::MEDPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    octaveOffset = 12;
    m_font = QFont("MED 3.21");
    m_font.setPixelSize(16);
    m_fontWidth = 16;

    m_bitmapFont = BitmapFont("MED 3.21");
    m_bitmapFont3 = BitmapFont("MED 3.21 Compressed");
    m_fontWidth = 16;
    m_fontHeight = 16;

    rowNumberOffset = 0;
    m_emptyInstrument = "0";
    instrumentPad = false;
    m_colorCurrentRowBackground = QColor(102, 102, 119);
    m_colorCurrentRowForeground = QColor(153, 153, 170);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(204, 204, 204);

    m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowEnd = "'";
    m_RowLength = 40;
    m_xOffsetRow = 8;

    m_xChannelStart = 56;
    m_channelWidth = 144;
    m_channelxSpace = 0;
    m_width = 640;
    m_height = 512;

    m_vumeterWidth = 24;
    m_vumeterHeight = 128;
    m_vumeterLeftOffset = 152;
    m_vumeterHilightWidth = 4;
    m_vumeterOffset = 144;
    m_vumeterTopOffset = -6;

    //main color
    m_linearGrad.setColorAt(0, QColor(187, 187, 187).rgb());
    m_linearGrad.setColorAt(0.045, QColor(187, 187, 187).rgb());
    m_linearGrad.setColorAt(0.04501, QColor(255, 34, 17).rgb()); //red
    m_linearGrad.setColorAt(0.46, QColor(255, 255, 51).rgb()); //yellow
    m_linearGrad.setColorAt(0.75, QColor(0, 255, 0).rgb()); // green
    m_linearGrad.setColorAt(1, QColor(0, 136, 0).rgb()); // dark green

    m_linearGradHiLite.setColorAt(0, QColor(187, 187, 187).rgb());
    m_linearGradHiLite.setColorAt(1, QColor(187, 187, 187).rgb());

    m_linearGradDark.setColorAt(0, QColor(187, 187, 187).rgb());
    m_linearGradDark.setColorAt(1, QColor(187, 187, 187).rgb());


}

BitmapFont MEDPatternView::infoFont()
{
    return m_bitmapFont3;
}

QString MEDPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber + rowNumberOffset, base).toUpper();
    rowNumberStr = rowNumberStr.length() == 1 ? "00" + rowNumberStr : rowNumberStr;
    rowNumberStr = rowNumberStr.length() == 2 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}

QString MEDPatternView::effect(BaseRow* row)
{
    if (row->effect == 171) return "F"; //might be a bug in libxmp
    if (row->effect == 146) return "4"; //might be a bug in libxmp
    return AbstractPatternView::effect(row);
}

QString MEDPatternView::note(BaseRow* row)
{
    //hold/decay
    if (row->effect2 == 177)
    {
        return "-|-";
    }
    QString note = AbstractPatternView::note(row);
    if (note.left(1) == "B")
    {
        note = "H" + note.right(2);
    }
    return note;
}

QString MEDPatternView::instrument(BaseRow* row)
{
    int instrument = row->sample;
    QString padding = m_SeparatorRowNumber;
    if (instrument >= 32)
    {
        padding = "";
    }
    return padding + QString::number(instrument, 32).toUpper();
}

void MEDPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(153, 153, 170);
    QColor colorHilite(204, 204, 204);
    QColor colorShadow(102, 102, 119);

    //bottom
    painter->fillRect(8, (height) - 8, 625, 2, colorHilite);
    painter->fillRect(6, (height) - 6, 628, 2, colorBase);
    painter->fillRect(4, (height) - 4, 633, 2, colorShadow);
    painter->fillRect(4, (height) - 2, 634, 2, colorShadow);

    //vumeter bottom
    for (unsigned int i = 0; i < m_channels; i++)
    {
        painter->fillRect((152) + 144 * i, (height) - (8), 4, 2, QColor(170, 170, 170));
        painter->fillRect((156) + 144 * i, (height) - (8), 16, 2, QColor(0, 136, 0));
        painter->fillRect((172) + 144 * i, (height) - (8), 4, 2, QColor(187, 187, 187));
    }
}

void MEDPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(153, 153, 170);
    QColor colorHilite(204, 204, 204);
    QColor colorShadow(102, 102, 119);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);

    painter->fillRect(8, (height / 2) - 17, 628, 18, m_colorCurrentRowBackground);

    //left border

    painter->fillRect(0, 0, 4, height, colorShadow);
    painter->fillRect(4, 0, 2, height, colorBase);
    painter->fillRect(6, 0, 2, height, colorHilite);


    //right border
    painter->fillRect(636, 0, 4, height, colorHilite);
    painter->fillRect(634, 0, 2, height, colorBase);
    painter->fillRect(632, 0, 2, height, colorShadow);
}

void::MEDPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    if(QString(info->fileformat.c_str()).toLower().startsWith("octamed (mmd0"))
    {
        m_height = 32;
        QColor colorBase(153, 153, 170);
        QColor colorHilite(204, 204, 204);
        QColor colorShadow(102, 102, 119);
        int left = 0;

        //left
        painter->fillRect(left, 0, 4, 32, colorShadow);
        painter->fillRect(left + 4, 4, 2, 28, colorBase);

        //right
        painter->fillRect(left + 636, 4, 4, 28, colorHilite);
        painter->fillRect(left + 634, 30, 2, 2, colorBase);

        //top
        painter->fillRect(left + 4, 4, 632, 26, colorBase);
        painter->fillRect(left + 1, 0, 639, 2, colorHilite);
        painter->fillRect(left + 3, 2, 637, 2, colorHilite);

        //bottom
        painter->fillRect(left + 6, 30, 1, 2, colorHilite);
        painter->fillRect(left + 7, 30, 627, 2, colorShadow);

        //position info etc.
        painter->fillRect(left + 6, 6, 154, 22, colorShadow);
        painter->fillRect(left + 7, 8, 83, 2, colorHilite);
        painter->fillRect(left + 91, 8, 67, 2, colorHilite);
        painter->fillRect(left + 88, 10, 2, 16, colorHilite);
        painter->fillRect(left + 156, 10, 2, 16, colorHilite);
        painter->fillRect(left + 89, 26, 1, 2, colorHilite);
        painter->fillRect(left + 157, 26, 1, 2, colorHilite);

        painter->fillRect(left + 8, 10, 80, 16, colorBase);
        painter->fillRect(left + 92, 10, 64, 16, colorBase);

        painter->setPen(QColor(0, 0, 0));
        drawText(QString("%1").arg(m_currentPosition + 1, 4, 10, QChar('0')), painter, left + 10, 24, infoFont());
        drawText("/", painter, left + 44, 24, infoFont());
        drawText(QString("%1").arg(info->numOrders, 4, 10, QChar('0')), painter, left + 54, 24, infoFont());
        drawText(QString("%1").arg(m_currentPattern, 3, 10, QChar('0')), painter, left + 94, 24, infoFont());
        drawText("/", painter, left + 120, 24, infoFont());
        drawText(QString("%1").arg(info->numPatterns, 3, 10, QChar('0')), painter, left + 130, 24, infoFont());
    }
    else
    {
        m_height = 28;
        QColor colorBase(153, 153, 170);
        QColor colorHilite(204, 204, 204);
        QColor colorShadow(102, 102, 119);
        int left = 0;

        //background
        painter->fillRect(left + 2, 0, 636, m_height, colorBase);

        //left
        painter->fillRect(left, 0, 2, m_height, colorShadow);


        //right
        painter->fillRect(left + 638, 2, 2, m_height, colorHilite);

        //top
        painter->fillRect(left + 1, 0, 639, 2, colorHilite);


        //bottom
        painter->fillRect(left + 1, 26, 638, 2, colorShadow);

        //position info etc.
        painter->fillRect(left + 3, 4, 253, 22, colorShadow);
        painter->fillRect(left + 4, 4, 83, 2, colorHilite);
        painter->fillRect(left + 88, 4, 67, 2, colorHilite);
        painter->fillRect(left + 156, 4, 35, 2, colorHilite);
        painter->fillRect(left + 192, 4, 64, 2, colorHilite);
        painter->fillRect(left + 85, 6, 2, 18, colorHilite);
        painter->fillRect(left + 153, 6, 2, 18, colorHilite);
        painter->fillRect(left + 189, 6, 2, 18, colorHilite);
        painter->fillRect(left + 254, 6, 2, 18, colorHilite);
        painter->fillRect(left + 86, 24, 1, 2, colorHilite);
        painter->fillRect(left + 154, 24, 1, 2, colorHilite);
        painter->fillRect(left + 190, 24, 1, 2, colorHilite);
        painter->fillRect(left + 255, 24, 1, 2, colorHilite);

        painter->fillRect(left + 5, 6, 80, 18, colorBase);
        painter->fillRect(left + 89, 6, 64, 18, colorBase);
        painter->fillRect(left + 157, 6, 32, 18, colorBase);
        painter->fillRect(left + 193, 6, 61, 18, colorBase);

        painter->setPen(QColor(0, 0, 0));

        drawText(QString("%1").arg(m_currentPosition + 1, 4, 10, QChar('0')), painter, left + 6, 24, infoFont());
        drawText("/", painter, left + 40, 24, infoFont());
        drawText("/", painter, left + 41, 24, infoFont());
        drawText(QString("%1").arg(info->numOrders, 4, 10, QChar('0')), painter, left + 50, 24, infoFont());
        drawText(QString("%1").arg(m_currentPattern, 3, 10, QChar('0')), painter, left + 90, 24, infoFont());
        drawText("/", painter, left + 116, 24, infoFont());
        drawText("/", painter, left + 117, 24, infoFont());
        drawText(QString("%1").arg(info->numPatterns, 3, 10, QChar('0')), painter, left + 126, 24, infoFont());
        drawText("SPD", painter, left + 161, 24, infoFont());
        drawText(QString("%1").arg(m_currentBPM, 3, 10, QChar('0')), painter, left + 196, 24, infoFont());
        drawText("/", painter, left + 223, 24, infoFont());
        drawText("/", painter, left + 224, 24, infoFont());
        drawText(QString("%1").arg(m_currentSpeed, 2, 16, QChar('0')), painter, left + 235, 24, infoFont());
    }
}
MEDPatternView::~MEDPatternView()
{
}
