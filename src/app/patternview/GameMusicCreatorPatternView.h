#ifndef GAMEMUSICCREATORPATTERNVIEW_H
#define GAMEMUSICCREATORPATTERNVIEW_H
#include "AbstractPatternView.h"

class GameMusicCreatorPatternView : public AbstractPatternView
{

public:
    GameMusicCreatorPatternView(Tracker *parent, unsigned int channels, int scale);
    ~GameMusicCreatorPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);


private:

private slots:

};

#endif // GAMEMUSICCREATORPATTERNVIEW_H
