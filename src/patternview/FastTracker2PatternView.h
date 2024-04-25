#ifndef FASTTRACKER2PATTERNVIEW_H
#define FASTTRACKER2PATTERNVIEW_H
#include "AbstractPatternView.h"

class FastTracker2PatternView : public AbstractPatternView
{

public:
    FastTracker2PatternView(Tracker *parent, unsigned int channels, int scale);
    ~FastTracker2PatternView();
    QFont fontEffects();
    int fontWidthEffects();
    int fontWidthInstrument();
    int fontWidthSeparatorNote();
    BitmapFont bitmapFontParameters();
    BitmapFont bitmapFontEffects();
    BitmapFont bitmapFontInstrument();
    BitmapFont infoFont();
    BitmapFont infoFont2();
    QFont fontParameters();
    QFont fontInstrument();
    int fontWidthParameters();
    QString note(BaseRow *row);
    QString effect(BaseRow *row);
    QString volume(BaseRow *row);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

protected:
    static const char* NOTES[121];
private slots:

};

#endif // FASTTRACKER2PATTERNVIEW_H
