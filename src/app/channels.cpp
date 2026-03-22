#include <QGridLayout>
#include "channels.h"
#include "mainwindow.h"
#include "plugins.h"
#include "soundmanager.h"

using uint128_t = unsigned __int128;

constexpr int maxChannels = 127;

Channels::Channels(MainWindow *mw, QWidget *parent) : QWidget(parent) {
    m_root = mw;
    const auto gridlayout = new QGridLayout(this);

    int row = 0;
    int col = 0;

    for (int i = 0; i < maxChannels; i++)
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

void Channels::updateChannelColors() const {
    for (int i = 0; i < maxChannels; i++)
    {
        channels.at(i)->setEnabledColor(QColor(m_root->colorMain.left(7)));
        channels.at(i)->setDisabledColor(QColor(m_root->colorMedium.left(7)));
        channels.at(i)->setTextColor(QColor(m_root->colorMainText.left(7)));
        channels.at(i)->setBackgroundColor(QColor(m_root->colorBehindBackground.left(7)));
    }
}

void Channels::updateChannels() const {
    const auto &info = SoundManager::getInstance().info;

    if (info == nullptr)
    {
        for (unsigned int i = 0; i < maxChannels; i++)
        {
            channels.at(i)->setVisible(false);
            channels.at(i)->update();
        }

        return;
    }

    for (unsigned int i = 0; i < maxChannels; i++)
    {
        channels.at(i)->setChecked(true);

        if (i < info->numChannels &&
            (info->plugin == PLUGIN_asap ||
             info->plugin == PLUGIN_game_music_emu ||
             info->plugin == PLUGIN_libsidplayfp ||
             info->plugin == PLUGIN_libopenmpt ||
             info->plugin == PLUGIN_libvgm ||
             info->plugin == PLUGIN_hivelytracker ||
             info->plugin == PLUGIN_furnace ||
             // info->plugin == PLUGIN_sndh_player ||
             info->plugin == PLUGIN_libxmp)) {
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

void Channels::setChannelEnabled(const int index, const bool enable) const {
    channels.at(index)->setChecked(enable);
    muteChannels();
}

bool Channels::getChannelEnabled(const int index) const {
    return channels.at(index)->isChecked();
}

void Channels::muteAllChannels() const {
    auto &sm = SoundManager::getInstance();
    const auto &info = sm.info;
    const unsigned int numChannels = info->numChannels;
    uint128_t mask = 0;
    QString maskStr = "";

    for (int i = 0; i < numChannels; i++)
    {
        mask |= 1 << i;
        maskStr += "0";
        channels.at(i)->setChecked(false);
    }

    if (info->plugin == PLUGIN_libopenmpt ||
        info->plugin == PLUGIN_libvgm ||
        info->plugin == PLUGIN_libxmp)
    {
        mask = 0;
    }

    sm.muteChannels(mask, maskStr);
}

void Channels::unmuteAllChannels() const {
    auto &sm = SoundManager::getInstance();
    const auto &info = sm.info;
    const unsigned int numChannels = info->numChannels;
    QString maskStr = "";

    for (int i = 0; i < numChannels; i++)
    {
        maskStr += "1";
        channels.at(i)->setChecked(true);
    }

    sm.muteChannels(0, maskStr);
}

void Channels::muteChannels() const {
    auto &sm = SoundManager::getInstance();
    const auto &info = sm.info;

    const unsigned int numChannels = info->numChannels;
    uint128_t mask = 0;
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

    if (info->plugin == PLUGIN_libopenmpt ||
        info->plugin == PLUGIN_libvgm ||
        info->plugin == PLUGIN_libxmp)
    {
        mask = 0;
    }

   sm.muteChannels(mask, maskStr);
}
