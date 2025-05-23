#include <QGridLayout>
#include "channels.h"
#include "mainwindow.h"
#include "plugins.h"
#include "soundmanager.h"

Channels::Channels(MainWindow* mw, QWidget* parent)
    : QWidget(parent)
{
    m_root = mw;
    QGridLayout* gridlayout = new QGridLayout(this);


    int row = 0;
    int col = 0;
    for (int i = 0; i < 64; i++)
    {
        channels.append(new ButtonOscilloscope(this, i));

        channels.at(i)->setVisible(false);
        channels.at(i)->setFocusPolicy(Qt::NoFocus);
        channels.at(i)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        channels.at(i)->setMinimumWidth(20);
        channels.at(i)->setMaximumWidth(16777215);
        channels.at(i)->setMinimumHeight(20);
        channels.at(i)->setMaximumHeight(16777215);

        channels.at(i)->setEnabledColor(QColor(m_root->colorMain.left(7)));
        channels.at(i)->setDisabledColor(QColor(m_root->colorMedium.left(7)));
        channels.at(i)->setTextColor(QColor(m_root->colorMainText.left(7)));
        channels.at(i)->setBackgroundColor(QColor(m_root->colorBehindBackground.left(7)));

        gridlayout->addWidget(channels.at(i), row, col);
        col++;
        if (col == 4)
        {
            col = 0;
            row++;
        }
    }

    this->setFocusPolicy(Qt::NoFocus);
    this->setGeometry(parent->geometry());
    parent->setLayout(gridlayout);
}

void Channels::updateChannelColors()
{
    for (int i = 0; i < 64; i++)
    {
        channels.at(i)->setEnabledColor(QColor(m_root->colorMain.left(7)));
        channels.at(i)->setDisabledColor(QColor(m_root->colorMedium.left(7)));
        channels.at(i)->setTextColor(QColor(m_root->colorMainText.left(7)));
        channels.at(i)->setBackgroundColor(QColor(m_root->colorBehindBackground.left(7)));
    }
}

void Channels::updateChannels()
{
    unsigned int numChannels;
    if (SoundManager::getInstance().m_Info1 == nullptr)
    {
        for (unsigned int i = 0; i < 64; i++)
        {
            channels.at(i)->setVisible(false);
            channels.at(i)->update();
        }
        return;
    }
    else
    {
        numChannels = SoundManager::getInstance().m_Info1->numChannels;
    }

    for (unsigned int i = 0; i < 64; i++)
    {
        channels.at(i)->setChecked(true);
        if (i < numChannels &&
            (SoundManager::getInstance().m_Info1->plugin == PLUGIN_asap ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_game_music_emu ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_libsidplayfp ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_libfc14audiodecoder ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_furnace ||
                // SoundManager::getInstance().m_Info1->plugin==PLUGIN_sndh_player ||
                SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp))
        {
            channels.at(i)->setVisible(true);
            channels.at(i)->update();
        }
        else
        {
            channels.at(i)->setVisible(false);
            channels.at(i)->update();
        }
    }
}

void Channels::setChannelEnabled(int index, bool enable)
{
    channels.at(index)->setChecked(enable);
    muteChannels();
}

bool Channels::getChannelEnabled(int index)
{
    return channels.at(index)->isChecked();
}

void Channels::muteAllChannels()
{
    unsigned int numChannels = SoundManager::getInstance().m_Info1->numChannels;
    unsigned int mask = 0;
    QString maskStr = "";
    for (int i = 0; i < numChannels; i++)
    {
        mask |= 1 << i;
        maskStr += "0";
        channels.at(i)->setChecked(false);
    }

    if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt || SoundManager::getInstance().m_Info1->plugin
        == PLUGIN_libxmp)
    {
        mask = 0;
    }
    SoundManager::getInstance().MuteChannels(mask, maskStr);
}

void Channels::unmuteAllChannels()
{
    unsigned int numChannels = SoundManager::getInstance().m_Info1->numChannels;
    QString maskStr = "";
    for (int i = 0; i < numChannels; i++)
    {
        maskStr += "1";
        channels.at(i)->setChecked(true);
    }

    SoundManager::getInstance().MuteChannels(0, maskStr);
}

void Channels::muteChannels()
{
    unsigned int numChannels = SoundManager::getInstance().m_Info1->numChannels;
    unsigned int mask = 0;
    QString maskStr = "";
    for (int i = 0; i < numChannels; i++)
    {
        if (!channels.at(i)->isChecked())
        {
            mask |= 1 << i;
            maskStr += "0";
        }
        else
        {
            maskStr += "1";
        }
    }

    if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt || SoundManager::getInstance().m_Info1->plugin
        == PLUGIN_libxmp)
    {
        mask = 0;
    }
    SoundManager::getInstance().MuteChannels(mask, maskStr);
}
