#include "fileinfoparser.h"
#include "soundmanager.h"
#include "various.h"
#include <QFileInfo>
#include <QPlainTextEdit>
#include "plugins.h"

const string piTitleTag = "Title";
const string piArtistTagPrio1 = "Artist";
const string piArtistTagPrio2 = "Album Artist";
const string piArtistTagPrio3 = "Composer";
const string piArtistTagPrio4 = "Author";

const unordered_map<string_view, int> metadataLookupMap = {
    {piTitleTag, 0},
    {piArtistTagPrio1, 1},
    {piArtistTagPrio2, 2},
    {piArtistTagPrio3, 3},
    {piArtistTagPrio4, 4},
    {"Album", 5},
    {"Date", 6},
    {"Track", 7},
    {"Compilation", 8},
    {"Disc", 9},
    {"Genre", 10},
    {"Copyright", 11},
    {"Publisher", 12},
    {"Encoded By", 13},
    {"Encoder", 14},
    {"Creation Time", 15},
    {"Gapless Playback", 16},
    {"Comment", 17}
};

void normalizeText(string &str) {
    bool isNewWord = true;

    for (char &c: str) {
        if (c == '_') {
            c = ' ';
        }
        if (c == ' ') {
            isNewWord = true;
        } else if (std::isalpha(c)) {
            c = isNewWord ? static_cast<const char>(toupper(c)) : static_cast<const char>(tolower(c));
            isNewWord = false;
        }
    }
}

FileInfoParser::FileInfoParser() = default;

void FileInfoParser::updateFileInfo(QTableWidget* tableInfo, const PlaylistItem* playlistItem)
{
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableInfo->clearContents();
    tableInfo->setRowCount(999);

    const QFileInfo fileinfo(playlistItem->fullPath);
    int row = 0;

    addInfo(tableInfo, &row, "Filename", fileinfo.fileName());
    addInfo(tableInfo, &row, "Path", fileinfo.path());
    addInfo(tableInfo, &row, "Size", groupDigits(fileinfo.size()) + " bytes");
    addInfo(tableInfo, &row, "Last Modified", fileinfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    addInfo(tableInfo, &row, "Created", fileinfo.birthTime().toString("yyyy-MM-dd hh:mm:ss"));
    addLengthInfo(tableInfo, playlistItem, &row);
    addInfo(tableInfo, &row, "Player Engine", SoundManager::getInstance().m_Info1->pluginName.c_str());
    addInfo(tableInfo, &row, "Format", SoundManager::getInstance().m_Info1->fileformat.c_str());
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
        {
            const int defaultSubSong = SoundManager::getInstance().m_Info1->defaultSubSong;
            addInfo(tableInfo, &row, "Default Subsong", defaultSubSong == -1 ? "-" : QString::number(defaultSubSong));
        }
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
            addInfo(tableInfo, &row, "Creation Date", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "POKEY Chip", SoundManager::getInstance().m_Info1->chips.c_str());
            addInfo(tableInfo, &row, "Replay Freq", SoundManager::getInstance().m_Info1->clockSpeedStr.c_str());
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
            addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            break;
        case PLUGIN_highly_experimental:
        case PLUGIN_highly_quixotic:
        case PLUGIN_highly_theoretical:
        case PLUGIN_lazyusf2:
        case PLUGIN_vio2sf:
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->genre));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            addInfo(tableInfo, &row, "Year", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
            addInfo(tableInfo, &row, "Volume", SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str());
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
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
        case PLUGIN_libkss:
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_libopenmpt:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Tracker",
                    fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->fileformatSpecific));
            addInfo(tableInfo, &row, "Container Format",
                    fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->containerFileformats));
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
            if (SoundManager::getInstance().m_Info1->fieldSet == 0) {
                const int defaultSubSong = SoundManager::getInstance().m_Info1->defaultSubSong;
                addInfo(tableInfo, &row, "Default Subsong",
                        defaultSubSong == 0 ? "-" : QString::number(defaultSubSong));

                addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
                addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
                addInfo(tableInfo, &row, "Released", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
            }

            addInfo(tableInfo, &row, "Compatibility", SoundManager::getInstance().m_Info1->compatibility.c_str());
            addInfo(tableInfo, &row, "SID Chip", SoundManager::getInstance().m_Info1->chips.c_str());
            addInfo(tableInfo, &row, "Clock Speed", SoundManager::getInstance().m_Info1->clockSpeedStr.c_str());

            if (SoundManager::getInstance().m_Info1->fieldSet == 0) {
                addInfo(tableInfo, &row, "Replayer", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
            } else {
                addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            }

            addInfo(tableInfo, &row, "Load Addr",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->loadAddr, 16));
            addInfo(tableInfo, &row, "Init Addr",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->initAddr, 16));
            addInfo(tableInfo, &row, "Play Addr",
                    "$" + QString::number(SoundManager::getInstance().m_Info1->playAddr, 16));

            if (SoundManager::getInstance().m_Info1->fieldSet == 0) {
                addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5.c_str());
            }
            break;
        case PLUGIN_libstsound:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            addInfo(tableInfo, &row, "Song Player", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->songPlayer));
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
            addInfo(tableInfo, &row, "Disk", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->disk));
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
            addInfo(tableInfo, &row, "MD5", SoundManager::getInstance().m_Info1->md5.c_str());
            break;
        case PLUGIN_libvgm:
            for (const string &field: *SoundManager::getInstance().m_Info1->allowedFields) {
                if (field == "TITLE")
                    addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
                if (field == "ARTIST")
                    addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist));
                if (field == "GAME")
                    addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game));
                if (field == "SYSTEM")
                    addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
                if (field == "EMULATOR")
                    addInfo(tableInfo, &row, "Emulator",
                            fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->emulator));
                if (field == "CHIPS")
                    addInfo(tableInfo, &row, "Chips", SoundManager::getInstance().m_Info1->chips.c_str());
                if (field == "DATE")
                    addInfo(tableInfo, &row, "Date", SoundManager::getInstance().m_Info1->date.c_str());
                if (field == "GENRE")
                    addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->genre));
                if (field == "ENCODED_BY")
                    addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->ripper));
                if (field == "COPYRIGHT" || field == "PUBLISHER")
                    addInfo(tableInfo, &row, "Copyright",
                            fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright));
                if (field == "COMMENT")
                    addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            }
            break;
        case PLUGIN_zxtune:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->author));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->system));
            addInfo(tableInfo, &row, "Container Files", SoundManager::getInstance().m_Info1->containerFilenames.c_str());
            addInfo(tableInfo, &row, "Container Formats", SoundManager::getInstance().m_Info1->containerFileformats.c_str());
            addInfo(tableInfo, &row, "Creation Date", SoundManager::getInstance().m_Info1->date.c_str());
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));
            addInfo(tableInfo, &row, "Loop Position",
                    QString::number(SoundManager::getInstance().m_Info1->loopPosition));
            addMultilineInfo(tableInfo, &row, "Comments", SoundManager::getInstance().m_Info1->comments);
            break;
        case PLUGIN_vgmstream: {
            addInfo(tableInfo, &row, "Encoding", SoundManager::getInstance().m_Info1->fileformatSpecific.c_str());
            addInfo(tableInfo, &row, "Sample Rate",
                    QString::number(SoundManager::getInstance().m_Info1->samplerate) + " Hz");

            const auto bitRate = SoundManager::getInstance().m_Info1->bitRate;
            addInfo(tableInfo, &row, "Bitrate", bitRate < 1000
                                                    ? QString::number(bitRate) + " bps"
                                                    : QString::number(bitRate / 1000) + " kbps");
            addInfo(tableInfo, &row, "Channels", QString::number(SoundManager::getInstance().m_Info1->numChannels));

            std::vector<std::pair<std::string, std::string> > metadataMain;
            std::vector<std::pair<std::string, std::string> > metadataOther;

            for (auto &metadata: SoundManager::getInstance().m_Info1->metadata) {
                auto metadataPolished = metadata;
                normalizeText(metadataPolished.first);

                if (metadataLookupMap.contains(metadataPolished.first)) {
                    metadataMain.push_back(metadataPolished);
                } else {
                    if (const string prefixToRemove = "id3v2_priv."; metadata.first.find(prefixToRemove) == 0) {
                        metadata.first.erase(0, prefixToRemove.length());
                    }

                    metadataOther.push_back(metadata);
                }
            }

            ranges::sort(metadataMain, [](const auto &a, const auto &b) {
                return metadataLookupMap.at(a.first) < metadataLookupMap.at(b.first);
            });

            metadataMain.insert(metadataMain.end(), metadataOther.begin(), metadataOther.end());

            bool isPiTitleSet = false;
            bool isPiArtistSet = false;

            for (auto const &[first, second]: metadataMain) {
                if (first == "Comment" || first == "Comments") {
                    addMultilineInfo(tableInfo, &row, first.c_str(), second);
                } else {
                    addInfo(tableInfo, &row, first.c_str(), fromUtf8OrLatin1(second));

                    if (!isPiTitleSet && first == piTitleTag) {
                        playlistItem->info->title = second;
                        isPiTitleSet = true;
                    }
                    if (!isPiArtistSet && (first == piArtistTagPrio1 || first == piArtistTagPrio2 ||
                                           first == piArtistTagPrio3 || first == piArtistTagPrio4)) {
                        playlistItem->info->artist = second;
                        isPiArtistSet = true;
                    }
                }
            }
        }
        break;
        case PLUGIN_fmod:
            showFmodSupportedTagsIfAny(tableInfo, playlistItem, &row);
            break;
        default: ;
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
    if (value.empty()) {
        addInfo(tableInfo, row, label, "");
    } else {
        const QString text = fromUtf8OrLatin1(value);
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
        song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_MS_REAL);
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
    const int currentSubsong = SoundManager::getInstance().m_Info1->currentSubsong;
    const int numSubsongs = SoundManager::getInstance().m_Info1->numSubsongs;
    const bool isSubsong = currentSubsong != 0 || numSubsongs > 1;

    addInfo(tableInfo, row, "Subsong",
            isSubsong ? QString::number(currentSubsong + 1) + "/" + QString::number(numSubsongs) : "-");
}

void FileInfoParser::showFmodSupportedTagsIfAny(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row) {
        FMOD_TAG tag;
        const int numTags = SoundManager::getInstance().getNumTags();

        for (int i = 0; i < numTags; i++)
        {
            if (const FMOD_RESULT res = SoundManager::getInstance().getTag(nullptr, i, &tag);
                res == FMOD_OK && tag.type == FMOD_TAGTYPE_SHOUTCAST && tag.datatype == FMOD_TAGDATATYPE_STRING) {
                auto tagName = QString(tag.name);
                QString tagData = static_cast<char *>(tag.data);

                if (tagName == "ARTIST")
                {
                    tagName = "Artist";
                    playlistItem->info->artist = tagData.toStdString();
                }
                else if (tagName == "TITLE")
                {
                    tagName = "Title";
                    playlistItem->info->title = tagData.toStdString();
                }
                else if (tagName == "icy-genre")
                {
                    tagName = "Genre";
                }
                else if (tagName == "icy-name")
                {
                    tagName = "Name";
                }
                else if (tagName == "icy-url")
                {
                    tagName = "URL";
                }
                else if (tagName == "icy-br")
                {
                    tagName = "Bitrate";
                }
                else if (tagName == "icy-pub")
                {
                    tagName = "Published";
                    tagData = tagData == "1" ? "Yes":"No";
                }

                if (!tagName.isEmpty())
                {
                    addInfo(tableInfo, row, tagName, tagData);
                }
            }
        }
}