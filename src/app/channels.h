#ifndef CHANNELS_H
#define CHANNELS_H

#include "buttonoscilloscope.h"

class MainWindow;

class Channels : public QWidget
{
    Q_OBJECT

public:
    Channels(MainWindow* mw, QWidget* parent);
    void updateChannels() const;
    void updateChannelColors() const;
    void setChannelEnabled(int index, bool enable) const;
    bool getChannelEnabled(int index) const;
    void muteAllChannels() const;
    void unmuteAllChannels() const;
    void muteChannels() const;

private:
    QVector<ButtonOscilloscope*> channels;
    MainWindow* m_root;
};

#endif // CHANNELS_H
