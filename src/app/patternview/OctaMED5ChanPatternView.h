#ifndef OCTAMED5CHANPATTERNVIEW_H
#define OCTAMED5CHANPATTERNVIEW_H
#include "MEDPatternView.h"

class OctaMED5ChanPatternView : public MEDPatternView
{
public:
    OctaMED5ChanPatternView(Tracker* parent, unsigned int channels, int scale);
    ~OctaMED5ChanPatternView();
    QString effect(BaseRow* row);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // OCTAMED5CHANPATTERNVIEW_H
