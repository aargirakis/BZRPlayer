#include <QPlainTextEdit>
#include "fileinfoparser.h"
#include "plugins.h"
#include "soundmanager.h"
#include "various.h"

using namespace std;

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
        } else if (isalpha(c)) {
            c = isNewWord ? static_cast<const char>(toupper(c)) : static_cast<const char>(tolower(c));
            isNewWord = false;
        }
    }
}

FileInfoParser::FileInfoParser() = default;

void FileInfoParser::updateFileInfo(QTableWidget *tableInfo, const PlaylistItem *playlistItem) {
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableInfo->clearContents();
    tableInfo->setRowCount(999);

    int row = 0;

    const auto &info = SoundManager::getInstance().info;

    if (info->isLocalFilePath) {
        addInfo(tableInfo, &row, "Filename", info->filename.c_str());
        addInfo(tableInfo, &row, "Path", info->fileDir.c_str());
        addInfo(tableInfo, &row, "Size", groupDigits(info->filesize) + " bytes");
        addInfo(tableInfo, &row, "Last Modified", info->fileLastModified.c_str());
        addInfo(tableInfo, &row, "Created", info->fileCreatedAt.c_str());
        addLengthInfo(tableInfo, playlistItem, &row);
    } else {
        addInfo(tableInfo, &row, "URL", info->filePath.c_str());
    }

    addInfo(tableInfo, &row, "Player Engine", info->pluginName.c_str());
    addInfo(tableInfo, &row, "Format", info->fileFormat.c_str());

    if (info->isLocalFilePath) {
        addSubsongInfo(tableInfo, &row);
    }

    switch (info->plugin) {
        case PLUGIN_adplug:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(info->artist));
            addMultilineInfo(tableInfo, &row, "Description", info->comments);
            addInfo(tableInfo, &row, "Patterns", QString::number(info->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(info->numOrders));
            break;
        case PLUGIN_asap: {
            const int defaultSubsong = info->defaultSubsong;
            addInfo(tableInfo, &row, "Default Subsong", defaultSubsong == -1 ? "-" : QString::number(defaultSubsong));
        }
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Creation Date", info->date.c_str());
            addInfo(tableInfo, &row, "POKEY Chip", info->chips.c_str());
            addInfo(tableInfo, &row, "Replay Freq", info->clockSpeedStr.c_str());
            break;
        case PLUGIN_furnace:
            addInfo(tableInfo, &row, "Name", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Album", fromUtf8OrLatin1(info->album));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(info->system));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addMultilineInfo(tableInfo, &row, "Notes", info->comments);
            break;
        case PLUGIN_game_music_emu:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(info->game));
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(info->copyright));
            addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(info->ripper));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(info->system));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            break;
        case PLUGIN_highly_experimental:
        case PLUGIN_highly_quixotic:
        case PLUGIN_highly_theoretical:
        case PLUGIN_lazyusf2:
        case PLUGIN_vio2sf:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(info->game));
            addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(info->genre));
            addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(info->copyright));
            addInfo(tableInfo, &row, "Year", info->date.c_str());
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(info->ripper));
            addInfo(tableInfo, &row, "Volume", info->volumeAmplificationStr.c_str());
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            break;
        case PLUGIN_hivelytracker:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Patterns", QString::number(info->numPatterns));
            addInfo(tableInfo, &row, "Pattern Length", QString::number(info->modPatternRows));
            break;
        case PLUGIN_klystron:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Pattern Rows", QString::number(info->numPatterns));
            break;
        case PLUGIN_libkss:
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            break;
        case PLUGIN_libopenmpt:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Tracker", fromUtf8OrLatin1(info->fileFormatSpecific));
            addInfo(tableInfo, &row, "Container Format", fromUtf8OrLatin1(info->containerFileformats));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Patters", QString::number(info->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(info->numOrders));
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            break;
        case PLUGIN_libpac:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Sheets", QString::number(info->numPatterns));
            addInfo(tableInfo, &row, "Positions", QString::number(info->numOrders));
            break;
        case PLUGIN_libsidplayfp:
            if (info->isSid) {
                const int defaultSubsong = info->defaultSubsong;
                addInfo(tableInfo, &row, "Default Subsong",
                        defaultSubsong == 0 ? "-" : QString::number(defaultSubsong));

                addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
                addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
                addInfo(tableInfo, &row, "Released", fromUtf8OrLatin1(info->copyright));
            }

            addInfo(tableInfo, &row, "Compatibility", info->compatibility.c_str());
            addInfo(tableInfo, &row, "SID Chip", info->chips.c_str());
            addInfo(tableInfo, &row, "Clock Speed", info->clockSpeedStr.c_str());

            if (info->isSid) {
                addInfo(tableInfo, &row, "Replayer", fromUtf8OrLatin1(info->songPlayer));
            } else {
                addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            }

            addInfo(tableInfo, &row, "Load Addr", "$" + QString::number(info->loadAddr, 16));
            addInfo(tableInfo, &row, "Init Addr", "$" + QString::number(info->initAddr, 16));
            addInfo(tableInfo, &row, "Play Addr", "$" + QString::number(info->playAddr, 16));

            if (info->isSid) {
                addInfo(tableInfo, &row, "MD5", info->md5.c_str());
            }
            break;
        case PLUGIN_libstsound:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            addInfo(tableInfo, &row, "Song Player", fromUtf8OrLatin1(info->songPlayer));
            break;
        case PLUGIN_libxmp:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Patters", QString::number(info->numPatterns));
            addInfo(tableInfo, &row, "Orders", QString::number(info->numOrders));
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            break;
        case PLUGIN_protrekkr:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            break;
        case PLUGIN_sc68:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Composer", fromUtf8OrLatin1(info->composer));
            addInfo(tableInfo, &row, "Disk", fromUtf8OrLatin1(info->album));
            addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(info->converter));
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(info->ripper));
            addInfo(tableInfo, &row, "Replay", info->replay.c_str());
            addInfo(tableInfo, &row, "Hardware", info->hardware.c_str());
            addInfo(tableInfo, &row, "Rate", QString::number(info->rate));
            addInfo(tableInfo, &row, "Address", "$" + QString::number(info->address, 16));
            break;
        case PLUGIN_sndh_player:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Artist", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "Year", info->date.c_str());
            addInfo(tableInfo, &row, "Clock Speed", QString::number(info->clockSpeed) + " Hz");
            addInfo(tableInfo, &row, "Ripper", fromUtf8OrLatin1(info->ripper));
            addInfo(tableInfo, &row, "Converter", fromUtf8OrLatin1(info->converter));
            break;
        case PLUGIN_uade:
            addInfo(tableInfo, &row, "MD5", info->md5.c_str());
            break;
        case PLUGIN_libvgm:
            for (const string &field: *info->allowedFields) {
                if (field == "TITLE")
                    addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
                if (field == "ARTIST")
                    addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
                if (field == "GAME")
                    addInfo(tableInfo, &row, "Game", fromUtf8OrLatin1(info->game));
                if (field == "SYSTEM")
                    addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(info->system));
                if (field == "EMULATOR")
                    addInfo(tableInfo, &row, "Emulator", fromUtf8OrLatin1(info->emulator));
                if (field == "CHIPS")
                    addInfo(tableInfo, &row, "Chips", info->chips.c_str());
                if (field == "DATE")
                    addInfo(tableInfo, &row, "Date", info->date.c_str());
                if (field == "GENRE")
                    addInfo(tableInfo, &row, "Genre", fromUtf8OrLatin1(info->genre));
                if (field == "ENCODED_BY")
                    addInfo(tableInfo, &row, "Dumper", fromUtf8OrLatin1(info->ripper));
                if (field == "COPYRIGHT" || field == "PUBLISHER")
                    addInfo(tableInfo, &row, "Copyright", fromUtf8OrLatin1(info->copyright));
                if (field == "COMMENT")
                    addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            }
            break;
        case PLUGIN_zxtune:
            addInfo(tableInfo, &row, "Title", fromUtf8OrLatin1(info->title));
            addInfo(tableInfo, &row, "Author", fromUtf8OrLatin1(info->artist));
            addInfo(tableInfo, &row, "System", fromUtf8OrLatin1(info->system));
            addInfo(tableInfo, &row, "Container Files", info->containerFilenames.c_str());
            addInfo(tableInfo, &row, "Container Formats", info->containerFileformats.c_str());
            addInfo(tableInfo, &row, "Creation Date", info->date.c_str());
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));
            addInfo(tableInfo, &row, "Loop Position", QString::number(info->loopPosition));
            addMultilineInfo(tableInfo, &row, "Comments", info->comments);
            break;
        case PLUGIN_vgmstream: {
            if (info->isTrackFromCueSheet) {
                addInfo(tableInfo, &row, "Track Filename", info->cueSheetTrackFilename.c_str());
            }

            addInfo(tableInfo, &row, "Encoding", info->fileFormatSpecific.c_str());
            addInfo(tableInfo, &row, "Sample Rate", QString::number(info->samplerate) + " Hz");

            const auto bitRate = info->bitRate;
            addInfo(tableInfo, &row, "Bitrate", bitRate < 1000
                                                    ? QString::number(bitRate) + " bps"
                                                    : QString::number(bitRate / 1000) + " kbps");
            addInfo(tableInfo, &row, "Channels", QString::number(info->numChannels));

            vector<pair<string, string> > metadataMain;
            vector<pair<string, string> > metadataOther;

            for (auto &metadata: info->metadata) {
                if (metadata.first.empty() ||
                    ranges::all_of(metadata.second, [](const unsigned char c) { return isspace(c); })) {
                    continue;
                }

                auto metadataPolished = metadata;
                normalizeText(metadataPolished.first);

                if (metadataLookupMap.contains(metadataPolished.first)) {
                    // handle metadata duplicates according to insertion order
                    if (ranges::find_if(metadataMain, [&metadataPolished](const auto &p) {
                        return p.first == metadataPolished.first;
                    }) == metadataMain.end()) {
                        metadataMain.push_back(metadataPolished);
                    }
                } else {
                    // handle metadata duplicates according to insertion order
                    if (ranges::find_if(metadataOther, [&metadata](const auto &p) {
                        return p.first == metadata.first;
                    }) == metadataOther.end()) {
                        if (const string prefixToRemove = "id3v2_priv."; metadata.first.find(prefixToRemove) == 0) {
                            metadata.first.erase(0, prefixToRemove.length());
                        }

                        metadataOther.push_back(metadata);
                    }
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

            if (!isPiTitleSet && info->isTrackFromCueSheet) {
                playlistItem->info->title = info->cueSheetTrackFilename;
            }
        }
        break;
        case PLUGIN_fmod:
            showFmodSupportedTagsIfAny(tableInfo, playlistItem, &row);
            break;
        default: ;
    }

    tableInfo->setRowCount(row);

    for (int i = 0; i < tableInfo->rowCount(); i++) {
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
    const auto &sm = SoundManager::getInstance();
    unsigned int songLengthMs = sm.getLength(FMOD_TIMEUNIT_MS);

    if (songLengthMs == 0 || songLengthMs == -1) {
        songLengthMs = sm.getLength(FMOD_TIMEUNIT_MS_REAL);
    }

    if (songLengthMs == 0) {
        songLengthMs = -1;
    }

    if (songLengthMs == -1 && playlistItem->length > 0) {
        songLengthMs = playlistItem->length;
    }

    addInfo(tableInfo, row, "Length", msToNiceStringExact(songLengthMs, true));
}

void FileInfoParser::addSubsongInfo(QTableWidget *tableInfo, int *row) {
    const auto &info = SoundManager::getInstance().info;

    const int currentSubsong = info->currentSubsong;
    const int numSubsongs = info->numSubsongs;
    const bool isSubsong = currentSubsong != 0 || numSubsongs > 1;

    addInfo(tableInfo, row, "Subsong",
            isSubsong ? QString::number(currentSubsong + 1) + "/" + QString::number(numSubsongs) : "-");
}

void FileInfoParser::showFmodSupportedTagsIfAny(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row) {
    FMOD_TAG tag;
    const auto &sm = SoundManager::getInstance();
    const int numTags = sm.getNumTags();

    QString title;
    QString artist;
    QString genre;
    QString name;
    QString bitrate;
    QString published;

    for (int i = 0; i < numTags; i++) {
        if (const FMOD_RESULT res = sm.getTag(nullptr, i, &tag);
            res != FMOD_OK || tag.type != FMOD_TAGTYPE_SHOUTCAST || tag.datatype != FMOD_TAGDATATYPE_STRING) {
            continue;
        }

        auto tagName = QString(tag.name);
        const QString tagData = static_cast<char *>(tag.data);

        if (tagName == "TITLE") {
            title = tagData;
        } else if (tagName == "ARTIST") {
            artist = tagData;
        } else if (tagName == "icy-genre") {
            genre = tagData;
        } else if (tagName == "icy-name") {
            name = tagData;
        } else if (tagName == "icy-br") {
            bitrate = tagData;
        } else if (tagName == "icy-pub") {
            published = tagData;
        }
    }

    if (!title.isEmpty()) {
        playlistItem->info->title = title.toStdString();
        addInfo(tableInfo, row, "Title", title);
    }
    if (!artist.isEmpty()) {
        playlistItem->info->artist = artist.toStdString();
        addInfo(tableInfo, row, "Artist", artist);
    }
    if (!genre.isEmpty()) {
        addInfo(tableInfo, row, "Genre", genre);
    }
    if (!name.isEmpty()) {
        addInfo(tableInfo, row, "Name", name);
    }
    if (!bitrate.isEmpty()) {
        addInfo(tableInfo, row, "Bitrate", bitrate);
    }
    if (!published.isEmpty()) {
        addInfo(tableInfo, row, "Published", published == "1" ? "Yes" : "No");
    }
}
