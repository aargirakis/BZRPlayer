#ifndef OCTAMED54CHANPATTERNVIEW_H
#define OCTAMED54CHANPATTERNVIEW_H
#include "MEDPatternView.h"

class OctaMED54ChanPatternView : public MEDPatternView
{

public:
    OctaMED54ChanPatternView(Tracker *parent, unsigned int channels, int scale);
    ~OctaMED54ChanPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);


private:

private slots:

};

#endif // OCTAMED54CHANPATTERNVIEW_H
