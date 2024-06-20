#ifndef CHANNELS_H
#define CHANNELS_H

#include "buttonoscilloscope.h"
#include <QWidget>

class MainWindow;
class Channels: public QWidget
{
    Q_OBJECT

public:
    Channels(MainWindow *mw, QWidget *parent);
    void updateChannels();
    void updateChannelColors();
    void setChannelEnabled(int index, bool enable);
    bool getChannelEnabled(int index);
    void muteAllChannels();
    void unmuteAllChannels();
    void muteChannels();
private:
    QVector<ButtonOscilloscope*> channels;
    MainWindow* m_root;


};

#endif // CHANNELS_H
