#ifndef OCTAMEDPATTERNVIEW_H
#define OCTAMEDPATTERNVIEW_H
#include "MEDPatternView.h"

class OctaMEDPatternView : public MEDPatternView
{
public:
    OctaMEDPatternView(Tracker* parent, unsigned int channels);
    ~OctaMEDPatternView();
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // OCTAMEDPATTERNVIEW_H
