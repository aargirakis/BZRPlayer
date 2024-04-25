#ifndef GENERICPATTERNVIEW_H
#define GENERICPATTERNVIEW_H
#include "AbstractPatternView.h"

class GenericPatternView : public AbstractPatternView
{

public:
    GenericPatternView(Tracker *parent, unsigned int channels, int scale);
    ~GenericPatternView();

    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:

};

#endif // GENERICPATTERNVIEW_H
