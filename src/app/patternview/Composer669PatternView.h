#ifndef COMPOSER669PATTERNVIEW_H
#define COMPOSER669PATTERNVIEW_H
#include "AbstractPatternView.h"

class Composer669PatternView : public AbstractPatternView
{
public:
    Composer669PatternView(Tracker* parent, unsigned int channels);
    ~Composer669PatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);
    QString note(BaseRow* row);
    QString volume(BaseRow* row);

private:


private slots:
};

#endif // COMPOSER669PATTERNVIEW_H
