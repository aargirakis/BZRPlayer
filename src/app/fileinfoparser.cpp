#include "fileinfoparser.h"
#include "qdatetime.h"
#include "soundmanager.h"
#include "various.h"
#include <QStringList>
#include <QFileInfo>
#include <QPlainTextEdit>
#include "plugins.h"

const string FileInfoParser::ID3V1_GENRES[id3v1GenresMax + 1] = {
    "Blues", // 0
    "Classic Rock", // 1
    "Country", // 2
    "Dance", // 3
    "Disco", // 4
    "Funk", // 5
    "Grunge", // 6
    "Hip Hop", // 7
    "Jazz", // 8
    "Metal", // 9
    "New Age", // 10
    "Oldies", // 11
    "Other", // 12
    "Pop", // 13
    "R&B", // 14
    "Rap", // 15
    "Reggae", // 16
    "Rock", // 17
    "Techno", // 18
    "Industrial", // 19
    "Alternative", // 20
    "Ska", // 21
    "Death Metal", // 22
    "Pranks", // 23
    "Soundtrack", // 24
    "Eurotechno", // 25
    "Ambient", // 26
    "Trip-Hop", // 27
    "Vocal", // 28
    "Jazz-Funk", // 29
    "Fusion", // 30
    "Trance", // 31
    "Classical", // 32
    "Instrumental", // 33
    "Acid", // 34
    "House", // 35
    "Game", // 36
    "Sound Clip", // 37
    "Gospel", // 38
    "Noise", // 39
    "Alternative Rock", // 40
    "Bass", // 41
    "Soul", // 42
    "Punk", // 43
    "Space", // 44
    "Meditative", // 45
    "Instrumental Pop", // 46
    "Instrumental Rock", // 47
    "Ethnic", // 48
    "Gothic", // 49
    "Dark Wave", // 50
    "Techno-Industrial", // 51
    "Electronic", // 52
    "Pop-Folk", // 53
    "Eurodance", // 54
    "Dream", // 55
    "Southern Rock", // 56
    "Comedy", // 57
    "Cult", // 58
    "Gangsta", // 59
    "Top 40", // 60
    "Christian Rap", // 61
    "Pop-Funk", // 62
    "Jungle", // 63
    "Native American", // 64
    "Cabaret", // 65
    "New Wave", // 66
    "Psychedelic", // 67
    "Rave", // 68
    "Showtunes", // 69
    "Trailer", // 70
    "Lo-Fi", // 71
    "Tribal", // 72
    "Acid Punk", // 73
    "Acid Jazz", // 74
    "Polka", // 75
    "Retro", // 76
    "Musical", // 77
    "Rock & Roll", // 78
    "Hard Rock", // 79
    "Folk", // 80
    "Folk Rock", // 81
    "National Folk", // 82
    "Swing", // 83
    "Fast Fusion", // 84
    "Bebop", // 85
    "Latin", // 86
    "Revival", // 87
    "Celtic", // 88
    "Bluegrass", // 89
    "Avant-Garde", // 90
    "Gothic Rock", // 91
    "Progressive Rock", // 92
    "Psychedelic Rock", // 93
    "Symphonic Rock", // 94
    "Slow Rock", // 95
    "Big Band", // 96
    "Chorus", // 97
    "Easy Listening", // 98
    "Acoustic", // 99
    "Humour", // 100
    "Speech", // 101
    "Chanson", // 102
    "Opera", // 103
    "Chamber Music", // 104
    "Sonata", // 105
    "Symphony", // 106
    "Booty Bass", // 107
    "Primus", // 108
    "Porn Groove", // 109
    "Satire", // 110
    "Slow Jam", // 111
    "Club", // 112
    "Tango", // 113
    "Samba", // 114
    "Folklore", // 115
    "Ballad", // 116
    "Power Ballad", // 117
    "Rhythmic Soul", // 118
    "Freestyle", // 119
    "Duet", // 120
    "Punk Rock", // 121
    "Drum Solo", // 122
    "A Cappella", // 123
    "Euro House", // 124
    "Dancehall", // 125
    "Goa", // 126
    "Drum & Bass", // 127
    "Club-House", // 128
    "Hardcore", // 129
    "Terror", // 130
    "Indie", // 131
    "Britpop", // 132
    "Worldbeat", // 133
    "Polsk Punk", // 134
    "Beat Music", // 135
    "Christian Gangsta Rap", // 136
    "Heavy Metal", // 137
    "Black Metal", // 138
    "Crossover", // 139
    "Contemporary Christian", // 140
    "Christian Rock", // 141
    "Merengue", // 142
    "Salsa", // 143
    "Thrash Metal", // 144
    "Anime", // 145
    "Jpop", // 146
    "Synth-Pop", // 147
    "Abstract", // 148
    "Art Rock", // 149
    "Baroque", // 150
    "Bhangra", // 151
    "Big Beat", // 152
    "Breakbeat", // 153
    "Chillout", // 154
    "Downtempo", // 155
    "Dub", // 156
    "EBM", // 157
    "Eclectic", // 158
    "Electro", // 159
    "Electroclash", // 160
    "Emo", // 161
    "Experimental", // 162
    "Garage", // 163
    "Global", // 164
    "IDM", // 165
    "Illbient", // 166
    "Industro-Goth", // 167
    "Jam Band", // 168
    "Krautrock", // 169
    "Leftfield", // 170
    "Lounge", // 171
    "Math Rock", // 172
    "New Romantic", // 173
    "Nu-Breakz", // 174
    "Post-Punk", // 175
    "Post-Rock", // 176
    "Psytrance", // 177
    "Shoegaze", // 178
    "Space Rock", // 179
    "Trop Rock", // 180
    "World Music", // 181
    "Neoclassical", // 182
    "Audiobook", // 183
    "Audio Theatre", // 184
    "Neue Deutsche Welle", // 185
    "Podcast", // 186
    "Indie Rock", // 187
    "G-Funk", // 188
    "Dubstep", // 189
    "Garage Rock", // 190
    "Psybient" // 191
};

FileInfoParser::FileInfoParser() = default;

void FileInfoParser::updateFileInfo(QTableWidget* tableInfo, PlaylistItem* playlistItem)
{
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableInfo->clearContents();
    tableInfo->setRowCount(999);

    QFileInfo fileinfo(playlistItem->fullPath);
    int row = 0;

    addInfo(tableInfo, &row, "Filename", fileinfo.fileName());
    addInfo(tableInfo, &row, "Path", fileinfo.path());
    addInfo(tableInfo, &row, "Size", groupDigits(fileinfo.size()) + " bytes");
    addInfo(tableInfo, &row, "Last Modified", fileinfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    addInfo(tableInfo, &row, "Created", fileinfo.birthTime().toString("yyyy-MM-dd hh:mm:ss"));
    addLengthInfo(tableInfo, playlistItem, &row);
    addInfo(tableInfo, &row, "Player Engine", SoundManager::getInstance().m_Info1->pluginName.c_str());
    addInfo(tableInfo, &row, "Type", SoundManager::getInstance().m_Info1->fileformat.c_str());
    addSubsongInfo(tableInfo, &row);

    switch (SoundManager::getInstance().m_Info1->plugin) {
        case PLUGIN_adplug:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addMultilineInfo(tableInfo, &row, "Description", SoundManager::getInstance().m_Info1->comments);
            addInfo(tableInfo, &row, "Patterns", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
            break;
        case PLUGIN_asap:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
            addInfo(tableInfo, &row, "Creation Date", SoundManager::getInstance().m_Info1->date.c_str());

            //TODO not handled
            if (SoundManager::getInstance().m_Info1->numChannels == 1) {
                addInfo(tableInfo, &row, "Channels", "Mono");
            } else if (SoundManager::getInstance().m_Info1->numChannels == 2) {
                addInfo(tableInfo, &row, "Channels", "Stereo");
            }

            addAsapClockSpeed(tableInfo, &row);
            break;
        case PLUGIN_furnace:
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            break;
        case PLUGIN_game_music_emu:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->dumper));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            break;
        case PLUGIN_highly_experimental:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        case PLUGIN_highly_quixotic:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        case PLUGIN_highly_theoretical:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        case PLUGIN_hivelytracker:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Patterns", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            addInfo(tableInfo, &row, "Pattern Length",
                    QString::number(SoundManager::getInstance().m_Info1->modPatternRows));
            break;
        case PLUGIN_klystron:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Pattern Rows", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            break;
        case PLUGIN_lazyusf2:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        case PLUGIN_libkss:
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_libopenmpt:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Patters", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_libpac:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Sheets", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            addInfo(tableInfo, &row, "Positions", QString::number(SoundManager::getInstance().m_Info1->numOrders));
            break;
        case PLUGIN_libsidplayfp:
            addInfo(tableInfo, &row, "SID Format", SoundManager::getInstance().m_Info1->fileformatSpecific.c_str());
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            addSidClockSpeed(tableInfo, &row);
            addSidCompatibility(tableInfo, &row);
            addInfo(tableInfo, &row, "Song Speed",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->songSpeed, 16));
            addInfo(tableInfo, &row, "SID Chip", SoundManager::getInstance().m_Info1->sidChip.c_str());
            addInfo(tableInfo, &row, "Load Address",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->loadAddr, 16));
            addInfo(tableInfo, &row, "Init Address",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->initAddr, 16));
            addInfo(tableInfo, &row, "Play Address",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->playAddr, 16));
            addInfo(tableInfo, &row, "Start Subsong",
                    QString::number(SoundManager::getInstance().m_Info1->startSubSong));
            addInfo(tableInfo, &row, "Replayer", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
            addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5New.c_str());
            addInfo(tableInfo, &row, "MD5 Old", SoundManager::getInstance().m_Info1->md5Old.c_str());
            break;
        case PLUGIN_libstsound:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            addInfo(tableInfo, &row, "Song Player", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
            addInfo(tableInfo, &row, "Song Type", SoundManager::getInstance().m_Info1->songType.c_str());
            break;
        case PLUGIN_libxmp:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Patters", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_protrekkr:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            break;
        case PLUGIN_sc68:
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
            addInfo(tableInfo, &row, "Composer", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->composer));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->converter));
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Replay", SoundManager::getInstance().m_Info1->replay.c_str());
            addInfo(tableInfo, &row, "Hardware", SoundManager::getInstance().m_Info1->hwname.c_str());
            addInfo(tableInfo, &row, "Rate", QString::number(SoundManager::getInstance().m_Info1->rate));
            addInfo(tableInfo, &row, "Address",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->address, 16));
            break;
        case PLUGIN_sndh_player:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Clock Speed",
                    QString::number(SoundManager::getInstance().m_Info1->clockSpeed) + " Hz");
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->converter));
            break;
        case PLUGIN_uade:
            addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5New.c_str());
            break;
        case PLUGIN_vgmplay_legacy:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Release date", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->dumper));
            addInfo(tableInfo, &row, "Chips used",
                    QString(SoundManager::getInstance().m_Info1->chips.c_str()).left(
                        QString(SoundManager::getInstance().m_Info1->chips.c_str()).length() - 2));
            addInfo(tableInfo, &row, "Version",
                    QString::number(SoundManager::getInstance().m_Info1->version >> 8) + "." +
                    QString::number(SoundManager::getInstance().m_Info1->version & 0xFF));
            addInfo(tableInfo, &row, "Gain", QString::number(SoundManager::getInstance().m_Info1->gain, 'f', 2));
            addInfo(tableInfo, &row, "Length", SoundManager::getInstance().m_Info1->loopInfo.c_str());
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_vio2sf:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        case PLUGIN_zxtune:
            // addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            // addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
            // addInfo(tableInfo, &row, "Program", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->replay));
            // addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            // addInfo(tableInfo, &row, "Patterns", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
            // addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
            // addInfo(tableInfo, &row, "Frames", QString::number(SoundManager::getInstance().m_Info1->numFrames));
            // addInfo(tableInfo, &row, "Loop Frame", QString::number(SoundManager::getInstance().m_Info1->loopFrame));
            // addInfo(tableInfo, &row, "Loop Position", QString::number(SoundManager::getInstance().m_Info1->loopPosition));
            // addInfo(tableInfo, &row, "Initial Tempo", QString::number(SoundManager::getInstance().m_Info1->initialTempo));
            // addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
            break;
        default: ;
    }

    showFmodSupportedTagsIfAny(tableInfo, playlistItem, &row);

    tableInfo->setRowCount(row);
    for (int i = 0; i < tableInfo->rowCount(); i++)
    {
        tableInfo->item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tableInfo->item(i, 1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void FileInfoParser::addInfo(QTableWidget *tableInfo, int *row, const QString &label, const QString &value) {
    tableInfo->setItem(*row, 0, new QTableWidgetItem(label));
    tableInfo->setItem((*row)++, 1, new QTableWidgetItem(value));
}

void FileInfoParser::addMultilineInfo(QTableWidget *tableInfo, int *row, const QString &label, const string &value) {
    if (!value.empty()) {
        QString text = fromUtf8OrLatin1(value);
        auto *plainText = new QPlainTextEdit(text);
        plainText->setReadOnly(true);
        plainText->setEnabled(true);
        tableInfo->setRowHeight(*row, 100);
        tableInfo->setCellWidget(*row, 1, plainText);
        addInfo(tableInfo, row, label, text);
    }
}

void FileInfoParser::addLengthInfo(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row) {
    unsigned int song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_MS);

    if (song_length_ms == 0 || song_length_ms == -1) {
        song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_SUBSONG_MS);
    }
    if (song_length_ms == 0) {
        song_length_ms = -1;
    }
    if (song_length_ms == -1 && playlistItem->length > 0) {
        song_length_ms = playlistItem->length;
    }

    addInfo(tableInfo, row, "Length", msToNiceStringExact(song_length_ms, true));
}

void FileInfoParser::addSubsongInfo(QTableWidget *tableInfo, int *row) {
    int currentSubsong = SoundManager::getInstance().m_Info1->currentSubsong;
    int numSubsongs = SoundManager::getInstance().m_Info1->numSubsongs;
    bool isSubsong = currentSubsong != 1 || numSubsongs != 1;

    addInfo(tableInfo, row, "Subsong",
            isSubsong ? QString::number(currentSubsong) + "/" + QString::number(numSubsongs) : "-");
}

void FileInfoParser::addSidClockSpeed(QTableWidget *tableInfo, int *row) {
    string clockSpeed;
    switch (SoundManager::getInstance().m_Info1->clockSpeed) {
        case 1: clockSpeed = "PAL";
            break;
        case 2: clockSpeed = "NTSC";
            break;
        case 3: clockSpeed = "Any";
            break;
        default: clockSpeed = "Unknown";
    }

    addInfo(tableInfo, row, "Clock Speed", clockSpeed.c_str());
}

void FileInfoParser::addSidCompatibility(QTableWidget *tableInfo, int *row) {
    string compatibility;
    switch (SoundManager::getInstance().m_Info1->compatibility) {
        case 0: compatibility = "C64 compatible";
            break;
        case 1: compatibility = "PSID specific";
            break;
        case 2: compatibility = "Real C64 only";
            break;
        case 3: compatibility = "Requires C64 Basic";
            break;
        default: compatibility = "Unknown";
    }

    addInfo(tableInfo, row, "Compatibility", compatibility.c_str());
}

void FileInfoParser::addAsapClockSpeed(QTableWidget *tableInfo, int *row) {
    string clockSpeed;
    switch (SoundManager::getInstance().m_Info1->clockSpeed) {
        case 0: clockSpeed = "NTSC";
            break;
        case 1: clockSpeed = "PAL";
            break;
        default: clockSpeed = "Unknown";
    }

    addInfo(tableInfo, row, "Clock Speed", clockSpeed.c_str());
}

void FileInfoParser::showFmodSupportedTagsIfAny(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row) {
        FMOD_TAG tag;
        FMOD_RESULT res;
        int numTags = SoundManager::getInstance().getNumTags();

        bool hasWrittenID3V1Header = false;
        bool hasWrittenID3V2Header = false;

        for (int i = 0; i < numTags; i++)
        {
            res = SoundManager::getInstance().getTag(0, i, &tag);
            if (res == FMOD_OK)
            {
                if (tag.type == FMOD_TAGTYPE_SHOUTCAST)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                    {
                        QString data;
                        data = static_cast<char *>(tag.data);
                        if (QString(tag.name) == "ARTIST")
                        {
                            tag.name = "Artist";
                            playlistItem->info->artist = data.toStdString();
                        }
                        else if (QString(tag.name) == "TITLE")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = data.toStdString();
                        }
                        else if (QString(tag.name) == "icy-genre")
                        {
                            tag.name = "Genre";
                        }
                        else if (QString(tag.name) == "icy-name")
                        {
                            tag.name = "Name";
                        }
                        else if (QString(tag.name) == "icy-url")
                        {
                            tag.name = "URL";
                        }
                        else if (QString(tag.name) == "icy-br")
                        {
                            tag.name = "Bitrate";
                        }
                        else if (QString(tag.name) == "icy-pub")
                        {
                            tag.name = "Published";
                            if (data == "1")
                            {
                                data = "Yes";
                            }
                            else
                            {
                                data = "No";
                            }
                        }
                        if (tag.name != "")
                        {
                            addInfo(tableInfo, row, tag.name, data);
                        }
                    }
                }
                else if (tag.type == FMOD_TAGTYPE_ID3V1)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                    {
                        if (!hasWrittenID3V1Header)
                        {
                            addInfo(tableInfo, row, "ID3v1 Tag", "");
                            hasWrittenID3V1Header = true;
                        }

                        QString DataAsString;

                        if (QString(tag.name) == "GENRE") {
                            tag.name = "Genre";

                            QString data = static_cast<char *>(tag.data);
                            bool isConversionOk;
                            unsigned int index = data.toUInt(&isConversionOk);

                            if (!isConversionOk || index > id3v1GenresMax) {
                                DataAsString = "";
                            } else {
                                DataAsString = ID3V1_GENRES[index].c_str();
                            }
                        } else {
                            DataAsString = QString::fromLatin1(static_cast<char *>(tag.data)).trimmed();

                            if (QString(tag.name) == "COMMENT") {
                                tag.name = "Comment";
                            } else if (QString(tag.name) == "ARTIST") {
                                tag.name = "Artist";
                                playlistItem->info->artist = DataAsString.toStdString();
                            } else if (QString(tag.name) == "TITLE") {
                                tag.name = "Title";
                                playlistItem->info->title = DataAsString.toStdString();
                            } else if (QString(tag.name) == "ALBUM") {
                                tag.name = "Album";
                            } else if (QString(tag.name) == "YEAR") {
                                tag.name = "Year";
                            } else if (QString(tag.name) == "TRACK") {
                                tag.name = "Track";
                            }
                        }

                        addInfo(tableInfo, row, tag.name, DataAsString);
                    }
                }
                else if (tag.type == FMOD_TAGTYPE_ID3V2)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            addInfo(tableInfo, row, "ID3v2 Tag", "");
                            hasWrittenID3V2Header = true;
                        }
                        QString data;
                        QString DataAsString = QString::fromUtf16(static_cast<ushort *>(tag.data));
                        if (QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                        {
                            tag.name = "Album Title";
                        }
                        else if (QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                        {
                            tag.name = "BPM";
                        }
                        else if (QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                        {
                            tag.name = "Composer";
                        }
                        else if (QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                        {
                            tag.name = "Content type";
                        }
                        else if (QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                        {
                            tag.name = "Copyright message";
                        }
                        else if (QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                        {
                            tag.name = "Date";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDEN")
                        {
                            tag.name = "Encoding time";
                        }
                        else if (QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                        {
                            tag.name = "Playlist delay";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDOR")
                        {
                            tag.name = "Original release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRC")
                        {
                            tag.name = "Recording time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRL")
                        {
                            tag.name = "Release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDTG")
                        {
                            tag.name = "Tagging time";
                        }
                        else if (QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                        {
                            tag.name = "Encoded by";
                        }
                        else if (QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                        {
                            tag.name = "Lyricist(s)/Text writer(s)";
                        }
                        else if (QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                        {
                            tag.name = "File type";
                        }
                        else if (QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                        {
                            tag.name = "Time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TIPL")
                        {
                            tag.name = "Involved people list";
                        }
                        else if (QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                        {
                            tag.name = "Content group description";
                        }
                        else if (QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                        {
                            tag.name = "Subtitle";
                        }
                        else if (QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                        {
                            tag.name = "Initial key";
                        }
                        else if (QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                        {
                            tag.name = "Language";
                        }
                        else if (QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                        {
                            tag.name = "Length";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMCL")
                        {
                            tag.name = "Musician credits list";
                        }
                        else if (QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                        {
                            tag.name = "Media type";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMOO")
                        {
                            tag.name = "Mood";
                        }
                        else if (QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                        {
                            tag.name = "Original album/movie/show title";
                        }
                        else if (QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                        {
                            tag.name = "Original filename";
                        }
                        else if (QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                        {
                            tag.name = "Original lyricist(s)/text writer(s)";
                        }
                        else if (QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                        {
                            tag.name = "Original artist(s)/performer(s)";
                        }
                        else if (QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                        {
                            tag.name = "Original Release Year";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TOWN")
                        {
                            tag.name = "File owner/licensee";
                        }
                        else if (QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                        {
                            tag.name = "Lead performer(s)/Soloist(s)";
                            playlistItem->info->artist = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                        {
                            tag.name = "Band/Orchestra/Accompaniment";
                        }
                        else if (QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                        {
                            tag.name = "Conductor";
                        }
                        else if (QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                        {
                            tag.name = "Interpreted, remixed, or otherwise modified by";
                        }
                        else if (QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                        {
                            tag.name = "Part of set";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TPRO")
                        {
                            tag.name = "Produced notice";
                        }
                        else if (QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                        {
                            tag.name = "Publisher";
                        }
                        else if (QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                        {
                            tag.name = "Track number";
                        }
                        else if (QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                        {
                            tag.name = "Recording dates";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSN")
                        {
                            tag.name = "Internet Radio Station Name";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSO")
                        {
                            tag.name = "Internet Radio Station Owner";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOA")
                        {
                            tag.name = "Album sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOP")
                        {
                            tag.name = "Performer sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOT")
                        {
                            tag.name = "Title sort order";
                        }
                        else if (QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                        {
                            tag.name = "Size";
                        }
                        else if (QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                        {
                            tag.name = "ISRC";
                        }
                        else if (QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                        {
                            tag.name = "Encoding Software";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSST")
                        {
                            tag.name = "Set subtitle";
                        }
                        else if (QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                        {
                            tag.name = "Year";
                        }

                        addInfo(tableInfo, row, tag.name, DataAsString);
                    }
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF8)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            addInfo(tableInfo, row, "ID3v2 Tag", "");
                            hasWrittenID3V2Header = true;
                        }
                        QString data;
                        data = static_cast<char *>(tag.data);
                        QString DataAsString = QString::fromUtf8(data.toStdString().c_str());
                        if (QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                        {
                            tag.name = "Album Title";
                        }
                        else if (QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                        {
                            tag.name = "BPM";
                        }
                        else if (QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                        {
                            tag.name = "Composer";
                        }
                        else if (QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                        {
                            tag.name = "Content type";
                        }
                        else if (QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                        {
                            tag.name = "Copyright message";
                        }
                        else if (QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                        {
                            tag.name = "Date";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDEN")
                        {
                            tag.name = "Encoding time";
                        }
                        else if (QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                        {
                            tag.name = "Playlist delay";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDOR")
                        {
                            tag.name = "Original release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRC")
                        {
                            tag.name = "Recording time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRL")
                        {
                            tag.name = "Release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDTG")
                        {
                            tag.name = "Tagging time";
                        }
                        else if (QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                        {
                            tag.name = "Encoded by";
                        }
                        else if (QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                        {
                            tag.name = "Lyricist(s)/Text writer(s)";
                        }
                        else if (QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                        {
                            tag.name = "File type";
                        }
                        else if (QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                        {
                            tag.name = "Time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TIPL")
                        {
                            tag.name = "Involved people list";
                        }
                        else if (QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                        {
                            tag.name = "Content group description";
                        }
                        else if (QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                        {
                            tag.name = "Subtitle";
                        }
                        else if (QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                        {
                            tag.name = "Initial key";
                        }
                        else if (QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                        {
                            tag.name = "Language";
                        }
                        else if (QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                        {
                            tag.name = "Length";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMCL")
                        {
                            tag.name = "Musician credits list";
                        }
                        else if (QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                        {
                            tag.name = "Media type";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMOO")
                        {
                            tag.name = "Mood";
                        }
                        else if (QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                        {
                            tag.name = "Original album/movie/show title";
                        }
                        else if (QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                        {
                            tag.name = "Original filename";
                        }
                        else if (QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                        {
                            tag.name = "Original lyricist(s)/text writer(s)";
                        }
                        else if (QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                        {
                            tag.name = "Original artist(s)/performer(s)";
                        }
                        else if (QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                        {
                            tag.name = "Original Release Year";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TOWN")
                        {
                            tag.name = "File owner/licensee";
                        }
                        else if (QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                        {
                            tag.name = "Lead performer(s)/Soloist(s)";
                            playlistItem->info->artist = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                        {
                            tag.name = "Band/Orchestra/Accompaniment";
                        }
                        else if (QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                        {
                            tag.name = "Conductor";
                        }
                        else if (QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                        {
                            tag.name = "Interpreted, remixed, or otherwise modified by";
                        }
                        else if (QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                        {
                            tag.name = "Part of set";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TPRO")
                        {
                            tag.name = "Produced notice";
                        }
                        else if (QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                        {
                            tag.name = "Publisher";
                        }
                        else if (QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                        {
                            tag.name = "Track number";
                        }
                        else if (QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                        {
                            tag.name = "Recording dates";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSN")
                        {
                            tag.name = "Internet Radio Station Name";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSO")
                        {
                            tag.name = "Internet Radio Station Owner";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOA")
                        {
                            tag.name = "Album sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOP")
                        {
                            tag.name = "Performer sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOT")
                        {
                            tag.name = "Title sort order";
                        }
                        else if (QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                        {
                            tag.name = "Size";
                        }
                        else if (QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                        {
                            tag.name = "ISRC";
                        }
                        else if (QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                        {
                            tag.name = "Encoding Software";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSST")
                        {
                            tag.name = "Set subtitle";
                        }
                        else if (QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                        {
                            tag.name = "Year";
                        }
                        addInfo(tableInfo, row, tag.name, DataAsString);
                    }
                    else if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            addInfo(tableInfo, row, "ID3v2 Tag", "");
                            hasWrittenID3V2Header = true;
                        }
                        QString data = QString::fromLatin1(static_cast<char*>(tag.data));
                        tag.name = const_cast<char*>(QString(tag.name).trimmed().toStdString().c_str());

                        if (QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                        {
                            tag.name = "Album Title";
                        }
                        else if (QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                        {
                            tag.name = "BPM";
                        }
                        else if (QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                        {
                            tag.name = "Composer";
                            playlistItem->info->composer = data.toStdString();
                        }
                        else if (QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                        {
                            tag.name = "Content type";
                        }
                        else if (QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                        {
                            tag.name = "Copyright message";
                        }
                        else if (QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                        {
                            tag.name = "Date";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDEN")
                        {
                            tag.name = "Encoding time";
                        }
                        else if (QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                        {
                            tag.name = "Playlist delay";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDOR")
                        {
                            tag.name = "Original release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRC")
                        {
                            tag.name = "Recording time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDRL")
                        {
                            tag.name = "Release time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TDTG")
                        {
                            tag.name = "Tagging time";
                        }
                        else if (QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                        {
                            tag.name = "Encoded by";
                        }
                        else if (QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                        {
                            tag.name = "Lyricist(s)/Text writer(s)";
                        }
                        else if (QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                        {
                            tag.name = "File type";
                        }
                        else if (QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                        {
                            tag.name = "Time";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TIPL")
                        {
                            tag.name = "Involved people list";
                        }
                        else if (QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                        {
                            tag.name = "Content group description";
                        }
                        else if (QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = data.toStdString();
                        }
                        else if (QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                        {
                            tag.name = "Subtitle";
                        }
                        else if (QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                        {
                            tag.name = "Initial key";
                        }
                        else if (QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                        {
                            tag.name = "Language";
                        }
                        else if (QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                        {
                            tag.name = "Length";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMCL")
                        {
                            tag.name = "Musician credits list";
                        }
                        else if (QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                        {
                            tag.name = "Media type";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TMOO")
                        {
                            tag.name = "Mood";
                        }
                        else if (QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                        {
                            tag.name = "Original album/movie/show title";
                        }
                        else if (QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                        {
                            tag.name = "Original filename";
                        }
                        else if (QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                        {
                            tag.name = "Original lyricist(s)/text writer(s)";
                        }
                        else if (QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                        {
                            tag.name = "Original artist(s)/performer(s)";
                            playlistItem->info->artist = data.toStdString();
                        }
                        else if (QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                        {
                            tag.name = "Original Release Year";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TOWN")
                        {
                            tag.name = "File owner/licensee";
                        }
                        else if (QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                        {
                            tag.name = "Lead performer(s)/Soloist(s)";
                            playlistItem->info->artist = data.toStdString();
                        }
                        else if (QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                        {
                            tag.name = "Band/Orchestra/Accompaniment";
                        }
                        else if (QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                        {
                            tag.name = "Conductor";
                        }
                        else if (QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                        {
                            tag.name = "Interpreted, remixed, or otherwise modified by";
                        }
                        else if (QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                        {
                            tag.name = "Part of set";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TPRO")
                        {
                            tag.name = "Produced notice";
                        }
                        else if (QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                        {
                            tag.name = "Publisher";
                        }
                        else if (QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                        {
                            tag.name = "Track number";
                        }
                        else if (QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                        {
                            tag.name = "Recording dates";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSN")
                        {
                            tag.name = "Internet Radio Station Name";
                        }
                        //does not exist in v2.0
                        else if (QString(tag.name) == "TRSO")
                        {
                            tag.name = "Internet Radio Station Owner";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOA")
                        {
                            tag.name = "Album sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOP")
                        {
                            tag.name = "Performer sort order";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSOT")
                        {
                            tag.name = "Title sort order";
                        }
                        else if (QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                        {
                            tag.name = "Size";
                        }
                        else if (QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                        {
                            tag.name = "ISRC";
                        }
                        else if (QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                        {
                            tag.name = "Encoding Software";
                        }
                        //id3v2.4
                        else if (QString(tag.name) == "TSST")
                        {
                            tag.name = "Set subtitle";
                        }
                        else if (QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                        {
                            tag.name = "Year";
                        }

                        if (!data.trimmed().isEmpty())
                        {
                            if (QString(tag.name) == "TXXX")
                            {
                                char* tst = static_cast<char *>(tag.data);
                                QString str;
                                for (int i = data.length(); i < tag.datalen; i++)
                                {
                                    if (tst[i] != 0) //don't add null values to string
                                    {
                                        str += tst[i];
                                    }
                                }

                                addInfo(tableInfo, row, data, str);

                                if (data == "replaygain_track_gain")
                                {
                                    float val = atof(str.trimmed().toStdString().c_str());
                                }
                            }
                            //iTunes Compilation Flag
                            else if (QString(tag.name) == "TCMP" || QString(tag.name) == "TCP")
                            {
                                tag.name = "Part of a compilation";
                                if (data == "1" || data.toLower() == "true" || data.toLower() == "yes")
                                {
                                    data = "Yes";
                                }
                                else
                                {
                                    data = "No";
                                }

                                addInfo(tableInfo, row, tag.name, data);

                            }
                            else if (QString(tag.name) == "Content type")
                            {
                                //match "(n)" where n is 0-999
                                QRegularExpression rx(QRegularExpression::anchoredPattern(QLatin1String("[(](\\d{1,3})[)]")));
                                QRegularExpressionMatch match = rx.match(data);
                                if (match.hasMatch())
                                {
                                    QStringList list;
                                    list << match.captured(1);
                                    bool isConversionOk;
                                    unsigned int index = list[0].toUInt(&isConversionOk);

                                    if (isConversionOk && index <= id3v1GenresMax) {
                                        data = ID3V1_GENRES[index].c_str();
                                    }
                                }
                                else if (data == "(RX)")
                                {
                                    data = "Remix";
                                }
                                else if (data == "(CR)")
                                {
                                    data = "Cover";
                                }

                                addInfo(tableInfo, row, tag.name, data);
                            }
                            else
                            {
                                addInfo(tableInfo, row, tag.name, data);
                            }
                        }
                    }
                }
                else if (tag.type == FMOD_TAGTYPE_VORBISCOMMENT)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF8)
                    {
                        QString data;
                        data = static_cast<char *>(tag.data);
                        QString DataAsString = QString::fromUtf8(data.toStdString().c_str());

                        if (QString(tag.name).toUpper() == "ACOUSTID_FINGERPRINT")
                        {
                            tag.name = "Acoustic fingerprint";
                        }

                        else if (QString(tag.name).toUpper() == "ACOUSTID_ID")
                        {
                            tag.name = "Acoustic ID";
                        }
                        else if (QString(tag.name).toUpper() == "ALBUM")
                        {
                            tag.name = "Album";
                        }
                        else if (QString(tag.name).toUpper() == "ALBUM ARTIST")
                        {
                            tag.name = "Album Artist";
                        }
                        else if (QString(tag.name).toUpper() == "ALBUMARTIST")
                        {
                            tag.name = "Album Artist";
                        }
                        else if (QString(tag.name).toUpper() == "ALBUMARTISTSORT")
                        {
                            tag.name = "Album Artist Sort";
                        }
                        else if (QString(tag.name).toUpper() == "ALBUMARTIST_CREDIT")
                        {
                            tag.name = "Album Artist Credit";
                        }
                        else if (QString(tag.name).toUpper() == "ARTIST")
                        {
                            tag.name = "Artist";
                            playlistItem->info->artist = data.toStdString();
                        }
                        else if (QString(tag.name).toUpper() == "ARTISTSORT")
                        {
                            tag.name = "Artist Sort";
                        }
                        else if (QString(tag.name).toUpper() == "ARTIST_CREDIT")
                        {
                            tag.name = "Artist Credit";
                        }
                        else if (QString(tag.name).toUpper() == "ASIN")
                        {
                            tag.name = "Amazon Standard Identification Number";
                        }
                        else if (QString(tag.name).toUpper() == "BPM")
                        {
                            tag.name = "BPM";
                        }
                        else if (QString(tag.name).toUpper() == "CATALOGNUMBER")
                        {
                            tag.name = "Catalog number";
                        }
                        else if (QString(tag.name).toUpper() == "COMPILATION")
                        {
                            tag.name = "Compilation";
                        }
                        else if (QString(tag.name).toUpper() == "COMPOSER")
                        {
                            tag.name = "Composer";
                        }
                        else if (QString(tag.name).toUpper() == "DATE")
                        {
                            tag.name = "Date";
                        }
                        else if (QString(tag.name).toUpper() == "DISC")
                        {
                            tag.name = "Disc";
                        }
                        else if (QString(tag.name).toUpper() == "DISCC")
                        {
                            tag.name = "Discc";
                        }
                        else if (QString(tag.name).toUpper() == "DISCNUMBER")
                        {
                            tag.name = "Disc number";
                        }
                        else if (QString(tag.name).toUpper() == "DISCSUBTITLE")
                        {
                            tag.name = "Disc subtitle";
                        }
                        else if (QString(tag.name).toUpper() == "DISCTOTAL")
                        {
                            tag.name = "Disc total";
                        }
                        else if (QString(tag.name).toUpper() == "ENCODEDBY")
                        {
                            tag.name = "Encoded by";
                        }
                        else if (QString(tag.name).toUpper() == "ENCODER")
                        {
                            tag.name = "Encoder";
                        }
                        else if (QString(tag.name).toUpper() == "GENRE")
                        {
                            tag.name = "Genre";
                        }
                        else if (QString(tag.name).toUpper() == "GROUPING")
                        {
                            tag.name = "Grouping";
                        }
                        else if (QString(tag.name).toUpper() == "LABEL")
                        {
                            tag.name = "Label";
                        }
                        else if (QString(tag.name).toUpper() == "LANGUAGE")
                        {
                            tag.name = "Language";
                        }
                        else if (QString(tag.name).toUpper() == "LYRICS")
                        {
                            tag.name = "Lyrics";
                        }
                        else if (QString(tag.name).toUpper() == "MEDIA")
                        {
                            tag.name = "Media";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMARTISTID")
                        {
                            tag.name = "Musicbrainz Album Artist ID";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMCOMMENT")
                        {
                            tag.name = "Musicbrainz Album Comment";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMID")
                        {
                            tag.name = "Musicbrainz Album ID";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMSTATUS")
                        {
                            tag.name = "Musicbrainz Album Status";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMTYPE")
                        {
                            tag.name = "Musicbrainz Album Type";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ARTISTID")
                        {
                            tag.name = "Musicbrainz Artist ID";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_RELEASEGROUPID")
                        {
                            tag.name = "Musicbrainz Release Group ID";
                        }
                        else if (QString(tag.name).toUpper() == "MUSICBRAINZ_TRACKID")
                        {
                            tag.name = "Musicbrainz Track ID";
                        }
                        else if (QString(tag.name).toUpper() == "ORIGINALDATE")
                        {
                            tag.name = "Original Date";
                        }
                        else if (QString(tag.name).toUpper() == "PUBLISHER")
                        {
                            tag.name = "Publisher";
                        }
                        else if (QString(tag.name).toUpper() == "RELEASECOUNTRY")
                        {
                            tag.name = "Release Country";
                        }
                        else if (QString(tag.name).toUpper() == "REPLAYGAIN_ALBUM_GAIN")
                        {
                            tag.name = "Replaygain Album Gain";
                        }
                        else if (QString(tag.name).toUpper() == "REPLAYGAIN_ALBUM_PEAK")
                        {
                            tag.name = "Replaygain Album Peak";
                        }
                        else if (QString(tag.name).toUpper() == "REPLAYGAIN_TRACK_GAIN")
                        {
                            tag.name = "Replaygain Track Gain";
                        }
                        else if (QString(tag.name).toUpper() == "REPLAYGAIN_TRACK_PEAK")
                        {
                            tag.name = "Replaygain Track Peak";
                        }
                        else if (QString(tag.name).toUpper() == "SCRIPT")
                        {
                            tag.name = "Script";
                        }
                        else if (QString(tag.name).toUpper() == "TITLE")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = DataAsString.toStdString();
                        }
                        else if (QString(tag.name).toUpper() == "TOTALDISCS")
                        {
                            tag.name = "Total Discs";
                        }
                        else if (QString(tag.name).toUpper() == "TOTALTRACKS")
                        {
                            tag.name = "Total Tracks";
                        }
                        else if (QString(tag.name).toUpper() == "TRACK")
                        {
                            tag.name = "Track";
                        }
                        else if (QString(tag.name).toUpper() == "TRACKC")
                        {
                            tag.name = "Total Tracks";
                        }
                        else if (QString(tag.name).toUpper() == "TRACKNUMBER")
                        {
                            tag.name = "Track number";
                        }
                        else if (QString(tag.name).toUpper() == "TRACKTOTAL")
                        {
                            tag.name = "Total Tracks";
                        }
                        else if (QString(tag.name).toUpper() == "YEAR")
                        {
                            tag.name = "Year";
                        }
                        else if (QString(tag.name).toUpper() == "ENCODER")
                        {
                            tag.name = "Encoder";
                        }
                        else if (QString(tag.name).toUpper() == "COPYRIGHT")
                        {
                            tag.name = "Copyright";
                        }
                        else if (QString(tag.name).toUpper() == "DESCRIPTION")
                        {
                            tag.name = "Desciption";
                        }
                        else if (QString(tag.name).toUpper() == "COMMENT")
                        {
                            tag.name = "Comment";
                        }
                        else if (QString(tag.name).toUpper() == "DISCID")
                        {
                            tag.name = "Disc ID";
                        }

                        addInfo(tableInfo, row, tag.name, QString::fromUtf8(data.toStdString().c_str()));
                    }
                }
            }
        }
}