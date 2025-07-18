#ifndef GAMEMUSICCREATORPATTERNVIEW_H
#define GAMEMUSICCREATORPATTERNVIEW_H
#include "AbstractPatternView.h"

class GameMusicCreatorPatternView : public AbstractPatternView
{
public:
    GameMusicCreatorPatternView(Tracker* parent, unsigned int channels);
    ~GameMusicCreatorPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // GAMEMUSICCREATORPATTERNVIEW_H
