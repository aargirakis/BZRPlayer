#ifndef OCTAMED44CHANPATTERNVIEW_H
#define OCTAMED44CHANPATTERNVIEW_H
#include "MEDPatternView.h"

class OctaMED44ChanPatternView : public MEDPatternView
{

public:
    OctaMED44ChanPatternView(Tracker *parent, unsigned int channels, int scale);
    ~OctaMED44ChanPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);


private:

private slots:

};

#endif // OCTAMED44CHANPATTERNVIEW_H
