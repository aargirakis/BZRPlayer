#ifndef ABSTRACTPATTERNVIEW_H
#define ABSTRACTPATTERNVIEW_H
#include <QPainter>
#include "BaseRow.h"
#include "bitmapfont.h"
#include "info.h"

class Tracker;
class AbstractPatternView
{


public:
    AbstractPatternView(Tracker *parent, unsigned int channels, int scale);



    virtual QString effect(BaseRow* row);
    virtual QString parameter(BaseRow* row);
    virtual QString effect2(BaseRow* row);
    virtual QString parameter2(BaseRow* row);
    virtual QString instrument(BaseRow* row);
    virtual QString volume(BaseRow* row);
    virtual QString rowNumber(int rowNumber);
    virtual QString note(BaseRow* row);
    virtual QFont font();
    virtual BitmapFont bitmapFont();
    virtual QFont currentRowFont();
    virtual BitmapFont currentRowBitmapFont();
    virtual BitmapFont infoFont();
    virtual BitmapFont infoFont2();
    virtual BitmapFont bitmapFontEffects();
    virtual BitmapFont bitmapFontInstrument();
    virtual BitmapFont bitmapFontParameters();
    virtual BitmapFont bitmapFontRownumber();

    virtual QFont fontEffects();
    virtual QFont fontInstrument();
    virtual QFont fontParameters();
    virtual QFont fontRownumber();

    virtual void paintAbove(QPainter* painter, int height, int currentRow);
    virtual void paintBelow(QPainter* painter, int height, int currentRow);
    virtual void paintTop(QPainter* painter, Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);
    virtual void drawText(QString text, QPainter* painter, int numPixels, int yPixelPosition, BitmapFont font, int letterSpacing=0);
    virtual void drawVerticalEmboss(int xPos, int yPos, int height, QColor hilite, QColor shadow, QColor base,
                                         QPainter* painter, bool left = true, bool right = true);

    virtual unsigned int height(){return m_height;}
    virtual unsigned int width(){return m_width;}
    virtual int scale(){return m_scale;}
    virtual QColor colorEmpty(){return m_colorEmpty;}
    virtual QColor colorDefault(){return m_colorDefault;}
    virtual QColor colorInstrument(){return m_ColorInstrument;}
    virtual QColor colorEffect(){return m_ColorEffect;}
    virtual QColor colorEffect2(){return m_ColorEffect2;}
    virtual QColor colorParameter(){return m_ColorParameter;}
    virtual QColor colorParameter2(){return m_ColorParameter2;}
    virtual QColor colorVolume(){return m_ColorVolume;}

    virtual QColor colorEmptyAlternate(){return m_colorEmptyAlternate;}
    virtual QColor colorDefaultAlternate(){return m_colorDefaultAlternate;}
    virtual QColor colorInstrumentAlternate(){return m_ColorInstrumentAlternate;}
    virtual QColor colorEffectAlternate(){return m_ColorEffectAlternate;}
    virtual QColor colorEffect2Alternate(){return m_ColorEffect2Alternate;}
    virtual QColor colorParameterAlternate(){return m_ColorParameterAlternate;}
    virtual QColor colorParameter2Alternate(){return m_ColorParameter2Alternate;}
    virtual QColor colorVolumeAlternate(){return m_ColorVolumeAlternate;}
    virtual int alternateChannelColorsFrequency(){return m_alternateChannelColorsFrequency;}

    virtual QColor colorRowNumber(){return m_ColorRowNumber;}
    virtual QColor colorRowNumberBackground(){return m_ColorRowNumberBackground;}
    virtual QColor colorRowHighlightBackground(){return m_ColorRowHighlightBackground;}
    virtual int highlightBackgroundOffset(){return m_highlightBackgroundOffset;}
    virtual int rowHighlightBackgroundFrequency(){return m_RowHighlightBackgroundFrequency;}
    virtual QColor colorRowHighlightForeground(){return m_colorRowHighlightForeground;}
    virtual QColor colorCurrentRowHighlightForeground(){return m_colorCurrentRowHighlightForeground;}
    virtual int rowHighlightForegroundFrequency(){return m_RowHighlightForegroundFrequency;}
    virtual QColor colorRowHighlightBackground2(){return m_ColorRowHighlightBackground2;}
    virtual int rowHighlightBackgroundFrequency2(){return m_RowHighlightBackgroundFrequency2;}
    virtual int colorRowNumberHighLightFrequency(){return m_ColorRowNumberHighLightFrequency;}
    virtual int xOffsetRow(){return m_xOffsetRow;}
    virtual int yOffsetCurrentRowAfter(){return m_yOffsetCurrentRowAfter;}
    virtual int yOffsetCurrentRowBefore(){return m_yOffsetCurrentRowBefore;}
    virtual int yOffsetRowAfter(){return m_yOffsetRowAfter;}
    virtual int xOffsetSeparatorRowNumber(){return m_xOffsetSeparatorRowNumber;}
    virtual QColor colorRowNumberHighLight(){return m_ColorRowNumberHighLight;}
    virtual QColor colorCurrentRowForeground(){return m_colorCurrentRowForeground;}
    virtual QColor colorCurrentRowBackground(){return m_colorCurrentRowBackground;}
    virtual QColor colorCurrentRowHighlightBackground(){return m_colorCurrentRowHighlightBackground;}
    virtual int currentRowHighlightBackgroundFrequency(){return m_CurrentRowHighlightBackgroundFrequency;}
    virtual int currentRowHighlightForegroundFrequency(){return m_CurrentRowHighlightForegroundFrequency;}
    virtual QString separatorInstrument(){return m_SeparatorInstrument;}
    virtual QString separatorVolume(){return m_SeparatorVolume;}
    virtual QString separatorChannel(){return m_SeparatorChannel;}
    virtual QString separatorRowNumber(){return m_SeparatorRowNumber;}
    virtual QString separatorRowNumberLast(){return m_SeparatorRowNumberLast;}
    virtual QString separatorNote(){return m_SeparatorNote;}
    virtual QString separatorEffect(){return m_SeparatorEffect;}
    virtual QString separatorEffect2(){return m_SeparatorEffect2;}
    virtual QString separatorParameter(){return m_SeparatorParameter;}
    virtual QString separatorParameter2(){return m_SeparatorParameter2;}
    virtual QString rowStart(){return m_RowStart;}
    virtual QString rowEnd(){return m_RowEnd;}
    virtual QString empty(){return m_empty;}
    virtual int fontHeight(){return m_fontHeight;}
    virtual int fontWidth(){return m_fontWidth;}
    virtual int fontWidthEffects(){return m_fontWidth;}
    virtual int fontWidthInstrument(){return m_fontWidth;}
    virtual int fontWidthRownumber(){return m_fontWidth;}
    virtual int fontWidthParameters(){return m_fontWidth;}
    virtual int fontWidthSeparatorNote(){return m_fontWidth;}
    virtual int rowLength(){return m_RowLength;}
    virtual QString colorWindowBackground() {return m_ColorWindowBackground;}
    virtual bool rowNumbersLastChannelEnabled(){return m_rowNumbersLastChannelEnabled;}
    virtual bool rowNumbersEveryChannelEnabled(){return m_RowNumbersEveryChannelEnabled;}
    virtual bool effectsThenParametersEnabled(){return m_effectsThenParametersEnabled;}
    virtual QColor colorRowNumberCurrentRow(){return m_ColorRowNumberCurrentRow;}
    virtual bool colorRowNumberCurrentRowEnabled(){return m_ColorRowNumberCurrentRowEnabled;}
    virtual int yOffsetRowHighlight() {return m_yOffsetRowHighlight;}
    virtual bool linesBetweenRows(){return m_linesBetweenRows;}
    virtual bool noEmptyInstrumentColor(){return m_noEmptyInstrumentColor;}
    virtual bool noEmptyVolumeColor(){return m_noEmptyVolumeColor;}
    virtual bool noEmptyEffectColor(){return m_noEmptyEffectColor;}
    virtual bool noEmptyEffect2Color(){return m_noEmptyEffect2Color;}
    virtual bool noEmptyParameterColor(){return m_noEmptyParameterColor;}
    virtual bool noEmptyParameter2Color(){return m_noEmptyParameter2Color;}
    virtual int useInstrumentColorOnEffectAndParameterFrequency(){return m_useInstrumentColorOnEffectAndParameterFrequency;}
    virtual int channelStart(){return m_xChannelStart;}
    virtual int channelWidth(){return m_channelWidth;}
    virtual int channelLastWidth(){return m_channelLastWidth==-1 ? m_channelWidth : m_channelLastWidth;}
    virtual int channelFirstWidth(){return m_channelFirstWidth==-1 ? m_channelWidth : m_channelFirstWidth;}
    virtual int channelxSpace(){return m_channelxSpace;}
    virtual QColor colorBackground(){return m_colorBackground;}
    virtual unsigned int getCurrentSample(){return m_iCurrentSample;}
    virtual void setCurrentSample(unsigned int sample){m_iCurrentSample = sample;}
    virtual unsigned int getbuttonPrevSampleWidth(){return m_ibuttonPrevSampleWidth;}
    virtual unsigned int getbuttonPrevSampleHeight(){return m_ibuttonPrevSampleHeight;}
    virtual unsigned int getbuttonPrevSampleX(){return m_ibuttonPrevSampleX;}
    virtual unsigned int getbuttonPrevSampleY(){return m_ibuttonPrevSampleY;}
    virtual unsigned int getbuttonNextSampleWidth(){return m_ibuttonNextSampleWidth;}
    virtual unsigned int getbuttonNextSampleHeight(){return m_ibuttonNextSampleHeight;}
    virtual unsigned int getbuttonNextSampleX(){return m_ibuttonNextSampleX;}
    virtual unsigned int getbuttonNextSampleY(){return m_ibuttonNextSampleY;}
    virtual int getChannelClicked(int x, int y);
    virtual unsigned int bottomFrameHeight(){return m_bottomFrameHeight;}
    virtual unsigned int topHeight(){return m_topHeight;}



    virtual ~AbstractPatternView();

    QString emptyNote(){return m_emptyNote;}
    QString emptyInstrument(){return m_emptyInstrument;}
    QString emptyEffect(){return m_emptyEffect;}
    QString emptyParameter(){return m_emptyParameter;}
    QString emptyVolume(){return m_emptyVolume;}
    Tracker* parent(){return m_trackerWindow;}

    BitmapFont m_bitmapFont;
    BitmapFont m_bitmapFont2;
    BitmapFont m_bitmapFont3;
    BitmapFont m_bitmapFont4;

    BitmapFont m_bitmapFontInstrument;
    BitmapFont m_bitmapFontEffects;
    BitmapFont m_bitmapFontParameters;
    BitmapFont m_bitmapFontRownumber;

protected:
    Tracker* m_trackerWindow;




    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_channels;
    unsigned int m_topHeight;
    unsigned int m_bottomFrameHeight;
    int m_scale;

    QPen m_pen;
    int m_fontWidth;
    int m_fontHeight;
    int m_fontWidthEffects;
    int m_fontWidthRownumber;
    int m_fontWidthParameters;
    int m_fontWidthSeparatorNote;
    int m_fontWidthInstrument;

    bool instrumentHex;
    bool instrumentPad;
    QString m_instrumentPaddingCharacter;
    bool effectHex;
    bool effectPad;
    bool volumeHex;
    bool volumePad;
    bool parameterPad;
    bool parameterHex;
    bool rowNumberPad;
    bool rowNumberHex;
    int rowNumberOffset;
    int instrumentOffset;
    int octaveOffset;
    bool volumeEnabled;
    bool instrumentEnabled;
    bool effect2Enabled;
    bool effectEnabled;
    bool parameterEnabled;
    bool parameter2Enabled;
    bool m_RowNumbersEveryChannelEnabled;
    bool m_rowNumbersLastChannelEnabled;
    bool m_effectsThenParametersEnabled;
    bool m_linesBetweenRows;


    QColor m_ColorRowNumber;
    QColor m_ColorRowNumberHighLight;
    QColor m_ColorRowNumberCurrentRow;
    bool m_ColorRowNumberCurrentRowEnabled;
    QColor m_ColorRowNumberBackground;
    QColor m_colorEmpty;
    QColor m_colorDefault;
    QColor m_ColorInstrument;
    QColor m_ColorEffect;
    QColor m_ColorEffect2;
    QColor m_ColorParameter;
    QColor m_ColorParameter2;
    QColor m_ColorVolume;
    QColor m_colorEmptyAlternate;
    QColor m_colorDefaultAlternate;
    QColor m_ColorInstrumentAlternate;
    QColor m_ColorEffectAlternate;
    QColor m_ColorEffect2Alternate;
    QColor m_ColorParameterAlternate;
    QColor m_ColorParameter2Alternate;
    QColor m_ColorVolumeAlternate;
    int m_alternateChannelColorsFrequency;
    QString m_ColorWindowBackground;
    QColor m_ColorRowHighlightBackground;
    int m_RowHighlightBackgroundFrequency;
    int m_ColorRowNumberHighLightFrequency;
    QColor m_ColorRowHighlightBackground2;
    int m_RowHighlightBackgroundFrequency2;
    int m_RowHighlightForegroundFrequency;
    QColor m_colorRowHighlightForeground;
    QColor m_colorCurrentRowHighlightBackground;
    QColor m_colorCurrentRowHighlightForeground;
    int m_highlightBackgroundOffset;
    int m_CurrentRowHighlightBackgroundFrequency;
    int m_CurrentRowHighlightForegroundFrequency;
    QString m_SeparatorInstrument;
    QString m_SeparatorRowNumberLast;
    QString m_SeparatorVolume;
    QString m_SeparatorChannel;
    QString m_SeparatorRowNumber;
    QString m_SeparatorNote;
    QString m_SeparatorEffect;
    QString m_SeparatorEffect2;
    QString m_SeparatorParameter;
    QString m_SeparatorParameter2;
    QString m_RowStart;
    QString m_RowEnd;

    QColor m_colorCurrentRowForeground;
    QColor m_colorCurrentRowBackground;

    QString m_emptyInstrument;
    QString m_emptyNote;
    QString m_emptyEffect;
    QString m_emptyParameter;
    QString m_emptyVolume;
    QString m_empty;

    //if true, do not use special empty color
    bool m_noEmptyInstrumentColor;
    bool m_noEmptyEffectColor;
    bool m_noEmptyParameterColor;
    bool m_noEmptyEffect2Color;
    bool m_noEmptyParameter2Color;
    bool m_noEmptyVolumeColor;

    int m_useInstrumentColorOnEffectAndParameterFrequency; // every n:th row effect and parameter have instrument color, only used in MultiTracker for now

    QFont m_font;
    QFont m_font2;
    QFont m_fontInstrument;
    QFont m_fontEffects;
    QFont m_fontParameters;
    QFont m_fontRownumber;

    int m_xOffsetRow;
    int m_yOffsetCurrentRowAfter;
    int m_yOffsetRowAfter;
    int m_yOffsetCurrentRowBefore;
    int m_yOffsetRowHighlight;
    int m_xOffsetSeparatorRowNumber;
    unsigned int m_RowLength;

    //used for enabling/disabling channels using the mouse
    int m_xChannelStart;
    int m_channelWidth;
    int m_channelFirstWidth;
    int m_channelLastWidth;
    int m_channelxSpace;
    QColor m_colorBackground;

    //used for browsing samples
    unsigned int m_iCurrentSample;
    unsigned int m_ibuttonPrevSampleWidth;
    unsigned int m_ibuttonPrevSampleHeight;
    unsigned int m_ibuttonPrevSampleX;
    unsigned int m_ibuttonPrevSampleY;
    unsigned int m_ibuttonNextSampleWidth;
    unsigned int m_ibuttonNextSampleHeight;
    unsigned int m_ibuttonNextSampleX;
    unsigned int m_ibuttonNextSampleY;

    static const char* NOTES[109];
};

#endif // ABSTRACTPATTERNVIEW_H
