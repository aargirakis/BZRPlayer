#include "fileinfoparser.h"
#include "fmod_errors.h"
#include "qdatetime.h"
#include "soundmanager.h"
#include "various.h"
#include <QStringList>
#include <QFileInfo>
#include <QPlainTextEdit>
#include "plugins.h"

const string FileInfoParser::ID3V1_GENRES[] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age",
    "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska",
    "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion",
    "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "AlternRock",
    "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic",
    "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult",
    "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret", "New Wave", "Psychadelic",
    "Rave", "Showtunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
    "Rock & Roll", "Hard Rock", "Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin",
    "Revival", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock",
    "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
    "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club",
    "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock",
    "Drum Solo", "Acapella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club - House", "Hardcore", "Terror",
    "Indie", "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal",
    "Crossover", "Contemporary Christian", "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
    "Synthpop"
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
    addInfo(tableInfo, &row, "Player Engine", SoundManager::getInstance().m_Info1->plugin == 0
                                                  ? PLUGIN_fmod_NAME
                                                  : SoundManager::getInstance().m_Info1->pluginName.c_str());
    addInfo(tableInfo, &row, "Type", SoundManager::getInstance().m_Info1->fileformat.c_str());
    addSubsongInfo(tableInfo, &row);

    if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_game_music_emu)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->dumper));
        addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_protrekkr)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libsidplayfp)
    {
        addInfo(tableInfo, &row, "SID Format", SoundManager::getInstance().m_Info1->fileformatSpecific.c_str());
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
        addSidClockSpeed(tableInfo, &row);
        addSidCompatibility(tableInfo, &row);
        addInfo(tableInfo, &row, "Song Speed",
                "$" + QString::number(SoundManager::getInstance().m_Info1->songSpeed, 16));
        addSidModel(tableInfo, &row);
        addInfo(tableInfo, &row, "Load Address",
                "$" + QString::number(SoundManager::getInstance().m_Info1->loadAddr, 16));
        addInfo(tableInfo, &row, "Init Address",
                "$" + QString::number(SoundManager::getInstance().m_Info1->initAddr, 16));
        addInfo(tableInfo, &row, "Play Address",
                "$" + QString::number(SoundManager::getInstance().m_Info1->playAddr, 16));
        addInfo(tableInfo, &row, "Start Subsong", QString::number(SoundManager::getInstance().m_Info1->startSubSong));
        addInfo(tableInfo, &row, "Replayer", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
        addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5New.c_str());
        addInfo(tableInfo, &row, "MD5 Old", SoundManager::getInstance().m_Info1->md5Old.c_str());
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_vgmplay_legacy)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Release date", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->dumper));
        addInfo(tableInfo, &row, "Chips used",
                QString(SoundManager::getInstance().m_Info1->chips.c_str()).left(
                    QString(SoundManager::getInstance().m_Info1->chips.c_str()).length() - 2));
        addInfo(tableInfo, &row, "Version", QString::number(SoundManager::getInstance().m_Info1->version >> 8) + "." +
                                            QString::number(SoundManager::getInstance().m_Info1->version & 0xFF));
        addInfo(tableInfo, &row, "Gain", QString::number(SoundManager::getInstance().m_Info1->gain, 'f', 2));
        addInfo(tableInfo, &row, "Length", SoundManager::getInstance().m_Info1->loopInfo.c_str());
        addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_webuade)
    {
        addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5New.c_str());
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
        addInfo(tableInfo, &row, "Patterns", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
        addInfo(tableInfo, &row, "Pattern Length",
                QString::number(SoundManager::getInstance().m_Info1->modPatternRows));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_klystron)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
        addInfo(tableInfo, &row, "Pattern Rows", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_furnace)
    {
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
        addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libpac)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
        addInfo(tableInfo, &row, "Sheets", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
        addInfo(tableInfo, &row, "Positions", QString::number(SoundManager::getInstance().m_Info1->numOrders));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_adplug)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addMultilineInfo(tableInfo, &row, "Description", SoundManager::getInstance().m_Info1->comments);
        addInfo(tableInfo, &row, "Patterns", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
        addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_asap)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
        addInfo(tableInfo, &row, "Creation Date", SoundManager::getInstance().m_Info1->date.c_str());

        //TODO not handled
        if (SoundManager::getInstance().m_Info1->numChannels == 1)
        {
            addInfo(tableInfo, &row, "Channels", "Mono");
        }
        else if (SoundManager::getInstance().m_Info1->numChannels == 2)
        {
            addInfo(tableInfo, &row, "Channels", "Stereo");
        }

        addAsapClockSpeed(tableInfo, &row);
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_experimental)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
        addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
    }
	else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_lazyusf2)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
        addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_theoretical)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
        addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_quixotic)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
        addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_vio2sf)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
        addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
        addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
        addInfo(tableInfo, &row, "Comments", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_zxtune)
    {
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
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libstsound)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
        addInfo(tableInfo, &row, "Song Player", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
        addInfo(tableInfo, &row, "Song Type", SoundManager::getInstance().m_Info1->songType.c_str());
    }

    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_sc68)
    {
        addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
        addInfo(tableInfo, &row, "Composer", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->composer));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->converter));
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Replay", SoundManager::getInstance().m_Info1->replay.c_str());
        addInfo(tableInfo, &row, "Hardware", SoundManager::getInstance().m_Info1->hwname.c_str());
        addInfo(tableInfo, &row, "Rate", QString::number(SoundManager::getInstance().m_Info1->rate));
        addInfo(tableInfo, &row, "Address", "$" + QString::number(SoundManager::getInstance().m_Info1->address, 16));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_sndh_player)
    {
        addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
        addInfo(tableInfo, &row, "Clock Speed",
                QString::number(SoundManager::getInstance().m_Info1->clockSpeed) + " Hz");
        addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
        addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->converter));
    }
    else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt)
    {
        addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
        addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
        addInfo(tableInfo, &row, "Patters", QString::number(SoundManager::getInstance().m_Info1->numPatterns));
        addInfo(tableInfo, &row, "Orders", QString::number(SoundManager::getInstance().m_Info1->numOrders));
        addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
    }
    else //use fmod to get the tag info
    {
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
                            tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                            tableInfo->setItem(row, 1, new QTableWidgetItem(data));
                            row++;
                        }
                    }
                }
                else if (tag.type == FMOD_TAGTYPE_ID3V1)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                    {
                        if (!hasWrittenID3V1Header)
                        {
                            tableInfo->setItem(row, 0, new QTableWidgetItem("ID3v1 Tag"));
                            tableInfo->setItem(row, 1, new QTableWidgetItem(""));
                            row++;
                            hasWrittenID3V1Header = true;
                        }
                        QString data;
                        QString DataAsString = QString::fromLatin1(static_cast<char*>(tag.data));
                        data = static_cast<char *>(tag.data);
                        if (QString(tag.name) == "GENRE")
                        {
                            tag.name = "Genre";
                            if (data.toInt() > 0 && data.toInt() < 128)
                            {
                                DataAsString = ID3V1_GENRES[data.toInt()].c_str();
                            }
                            else
                            {
                                DataAsString = "";
                            }
                        }
                        else if (QString(tag.name) == "ARTIST")
                        {
                            tag.name = "Artist";
                            playlistItem->info->artist = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "TITLE")
                        {
                            tag.name = "Title";
                            playlistItem->info->title = DataAsString.toStdString();
                        }
                        else if (QString(tag.name) == "COMMENT")
                        {
                            tag.name = "Comment";
                        }
                        else if (QString(tag.name) == "ALBUM")
                        {
                            tag.name = "Album";
                        }
                        else if (QString(tag.name) == "YEAR")
                        {
                            tag.name = "Year";
                        }
                        else if (QString(tag.name) == "TRACK")
                        {
                            tag.name = "Track";
                        }
                        tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                        tableInfo->setItem(row, 1, new QTableWidgetItem(DataAsString));
                        row++;
                    }
                }
                else if (tag.type == FMOD_TAGTYPE_ID3V2)
                {
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            tableInfo->setItem(row, 0, new QTableWidgetItem("ID3v2 Tag"));
                            tableInfo->setItem(row, 1, new QTableWidgetItem(""));
                            row++;
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

                        tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                        tableInfo->setItem(row, 1, new QTableWidgetItem(DataAsString));
                        row++;
                    }
                    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF8)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            tableInfo->setItem(row, 0, new QTableWidgetItem("ID3v2 Tag"));
                            tableInfo->setItem(row, 1, new QTableWidgetItem(""));
                            row++;
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

                        tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                        tableInfo->setItem(row, 1, new QTableWidgetItem(DataAsString));
                        row++;
                    }
                    else if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                    {
                        if (!hasWrittenID3V2Header)
                        {
                            tableInfo->setItem(row, 0, new QTableWidgetItem("ID3v2 Tag"));
                            tableInfo->setItem(row, 1, new QTableWidgetItem(""));
                            row++;
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
                                tableInfo->setItem(row, 0, new QTableWidgetItem(data));
                                char* tst = static_cast<char *>(tag.data);
                                QString str;
                                for (int i = data.length(); i < tag.datalen; i++)
                                {
                                    if (tst[i] != 0) //don't add null values to string
                                    {
                                        str += tst[i];
                                    }
                                }
                                tableInfo->setItem(row, 1, new QTableWidgetItem(str));
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
                                tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                                tableInfo->setItem(row, 1, new QTableWidgetItem(data));
                            }
                            else if (QString(tag.name) == "Content type")
                            {
                                tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                                //match "(n)" where n is 0-999
                                QRegularExpression rx(QRegularExpression::anchoredPattern(QLatin1String("[(](\\d{1,3})[)]")));
                                QRegularExpressionMatch match = rx.match(data);
                                if (match.hasMatch())
                                {
                                    QStringList list;
                                    list << match.captured(1);
                                    int number = list[0].toInt();
                                    //check if number is 0-147
                                    if (number >= 0 && number <= 147)
                                    {
                                        tableInfo->setItem(row, 1, new QTableWidgetItem(ID3V1_GENRES[number].c_str()));
                                    }
                                    else
                                    {
                                        tableInfo->setItem(row, 1, new QTableWidgetItem(data));
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
                                else
                                {
                                    tableInfo->setItem(row, 1, new QTableWidgetItem(data));
                                }
                            }
                            else
                            {
                                tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                                tableInfo->setItem(row, 1, new QTableWidgetItem(data));
                            }

                            row++;
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
                        tableInfo->setItem(row, 0, new QTableWidgetItem(tag.name));
                        tableInfo->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(data.toStdString().c_str())));
                        row++;
                    }
                }
            }
        }
    }


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

    if (song_length_ms == 0 || song_length_ms == 0xffffffff) {
        song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_SUBSONG_MS);
    }
    if (song_length_ms == 0) {
        song_length_ms = 0xffffffff;
    }
    if (song_length_ms == 0xffffffff && playlistItem->length > 0) {
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

void FileInfoParser::addSidModel(QTableWidget *tableInfo, int *row) {
    string sidModel;
    switch (SoundManager::getInstance().m_Info1->sidModel) {
        case 1: sidModel = "6581";
            break;
        case 2: sidModel = "8580";
            break;
        case 3: sidModel = "Any";
            break;
        default: sidModel = "Unknown";
    }

    addInfo(tableInfo, row, "SID Model", sidModel.c_str());
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
