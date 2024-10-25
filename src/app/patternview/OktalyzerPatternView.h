#ifndef OKTALYZERPATTERNVIEW_H
#define OKTALYZERPATTERNVIEW_H
#include "AbstractPatternView.h"

class OktalyzerPatternView : public AbstractPatternView
{
public:
    OktalyzerPatternView(Tracker* parent, unsigned int channels, int scale);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString instrument(BaseRow* row);
    ~OktalyzerPatternView();

private:

private slots:
};

#endif // OKTALYZERPATTERNVIEW_H
