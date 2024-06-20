#ifndef SOUNDFXPATTERNVIEW_H
#define SOUNDFXPATTERNVIEW_H
#include "AbstractPatternView.h"

class SoundFXPatternView : public AbstractPatternView
{

public:
    SoundFXPatternView(Tracker *parent, unsigned int channels, int scale);
    ~SoundFXPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString note(BaseRow* row);


private:


private slots:

};

#endif // SOUNDFXPATTERNVIEW_H
