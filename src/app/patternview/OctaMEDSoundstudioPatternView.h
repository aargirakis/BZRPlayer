#ifndef OCTAMEDSOUNDSTUDIOPATTERNVIEW_H
#define OCTAMEDSOUNDSTUDIOPATTERNVIEW_H
#include "AbstractPatternView.h"

class OctaMEDSoundstudioPatternView : public AbstractPatternView
{
public:
    OctaMEDSoundstudioPatternView(Tracker* parent, unsigned int channels, int scale);
    ~OctaMEDSoundstudioPatternView();
    BitmapFont infoFont();
    QString instrument(BaseRow* row);
    QString rowNumber(int rowNumber);
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // OCTAMEDSOUNDSTUDIOPATTERNVIEW_H
