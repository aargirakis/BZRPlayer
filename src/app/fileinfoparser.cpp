#include "fileinfoparser.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "qdatetime.h"
#include "soundmanager.h"
#include "various.h"
#include <QStringList>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QString>
#include "plugins.h"

const string FileInfoParser::ID3V1_GENRES[] = {"Blues"
                                           ,"Classic Rock"
                                           ,"Country"
                                           ,"Dance"
                                           ,"Disco"
                                           ,"Funk"
                                           ,"Grunge"
                                           ,"Hip-Hop"
                                           ,"Jazz"
                                           ,"Metal"
                                           ,"New Age"
                                           ,"Oldies"
                                           ,"Other"
                                           ,"Pop"
                                           ,"R&B"
                                           ,"Rap"
                                           ,"Reggae"
                                           ,"Rock"
                                           ,"Techno"
                                           ,"Industrial"
                                           ,"Alternative"
                                           ,"Ska"
                                           ,"Death Metal"
                                           ,"Pranks"
                                           ,"Soundtrack"
                                           ,"Euro-Techno"
                                           ,"Ambient"
                                           ,"Trip-Hop"
                                           ,"Vocal"
                                           ,"Jazz+Funk"
                                           ,"Fusion"
                                           ,"Trance"
                                           ,"Classical"
                                           ,"Instrumental"
                                           ,"Acid"
                                           ,"House"
                                           ,"Game"
                                           ,"Sound Clip"
                                           ,"Gospel"
                                           ,"Noise"
                                           ,"AlternRock"
                                           ,"Bass"
                                           ,"Soul"
                                           ,"Punk"
                                           ,"Space"
                                           ,"Meditative"
                                           ,"Instrumental Pop"
                                           ,"Instrumental Rock"
                                           ,"Ethnic"
                                           ,"Gothic"
                                           ,"Darkwave"
                                           ,"Techno-Industrial"
                                           ,"Electronic"
                                           ,"Pop-Folk"
                                           ,"Eurodance"
                                           ,"Dream"
                                           ,"Southern Rock"
                                           ,"Comedy"
                                           ,"Cult"
                                           ,"Gangsta"
                                           ,"Top 40"
                                           ,"Christian Rap"
                                           ,"Pop/Funk"
                                           ,"Jungle"
                                           ,"Native American"
                                           ,"Cabaret"
                                           ,"New Wave"
                                           ,"Psychadelic"
                                           ,"Rave"
                                           ,"Showtunes"
                                           ,"Trailer"
                                           ,"Lo-Fi"
                                           ,"Tribal"
                                           ,"Acid Punk"
                                           ,"Acid Jazz"
                                           ,"Polka"
                                           ,"Retro"
                                           ,"Musical"
                                           ,"Rock & Roll"
                                           ,"Hard Rock"
                                           ,"Folk"
                                           ,"Folk-Rock"
                                           ,"National Folk"
                                           ,"Swing"
                                           ,"Fast Fusion"
                                           ,"Bebob"
                                           ,"Latin"
                                           ,"Revival"
                                           ,"Celtic"
                                           ,"Bluegrass"
                                           ,"Avantgarde"
                                           ,"Gothic Rock"
                                           ,"Progressive Rock"
                                           ,"Psychedelic Rock"
                                           ,"Symphonic Rock"
                                           ,"Slow Rock"
                                           ,"Big Band"
                                           ,"Chorus"
                                           ,"Easy Listening"
                                           ,"Acoustic"
                                           ,"Humour"
                                           ,"Speech"
                                           ,"Chanson"
                                           ,"Opera"
                                           ,"Chamber Music"
                                           ,"Sonata"
                                           ,"Symphony"
                                           ,"Booty Bass"
                                           ,"Primus"
                                           ,"Porn Groove"
                                           ,"Satire"
                                           ,"Slow Jam"
                                           ,"Club"
                                           ,"Tango"
                                           ,"Samba"
                                           ,"Folklore"
                                           ,"Ballad"
                                           ,"Power Ballad"
                                           ,"Rhythmic Soul"
                                           ,"Freestyle"
                                           ,"Duet"
                                           ,"Punk Rock"
                                           ,"Drum Solo"
                                           ,"Acapella"
                                           ,"Euro-House"
                                           ,"Dance Hall"
                                           ,"Goa"
                                           ,"Drum & Bass"
                                           ,"Club - House"
                                           ,"Hardcore"
                                           ,"Terror"
                                           ,"Indie"
                                           ,"BritPop"
                                           ,"Negerpunk"
                                           ,"Polsk Punk"
                                           ,"Beat"
                                           ,"Christian Gangsta Rap"
                                           ,"Heavy Metal"
                                           ,"Black Metal"
                                           ,"Crossover"
                                           ,"Contemporary Christian"
                                           ,"Christian Rock"
                                           ,"Merengue"
                                           ,"Salsa"
                                           ,"Thrash Metal"
                                           ,"Anime"
                                           ,"JPop"
                                           ,"Synthpop"};




FileInfoParser::FileInfoParser()
{

}
void FileInfoParser::updateFileInfo(QTableWidget* tableInfo,PlaylistItem* playlistItem)
{
    unsigned int song_length_ms;
    song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_MS);

    if(song_length_ms==0 ||song_length_ms==0xffffffff)
    {
        song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_SUBSONG_MS);

    }

    if(song_length_ms==0)
    {
        song_length_ms=0xffffffff;
    }
    if(song_length_ms==0xffffffff && playlistItem->length>0)
    {
        song_length_ms=playlistItem->length;
    }
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    int row=0;
    tableInfo->clearContents();

    tableInfo->setRowCount(999);

    QFileInfo fileinfo(playlistItem->fullPath);
    tableInfo->setItem(row,0,new QTableWidgetItem("Filename"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(fileinfo.fileName()));
    tableInfo->setItem(row,0,new QTableWidgetItem("Path"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(fileinfo.path()));
    tableInfo->setItem(row,0,new QTableWidgetItem("Size"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(groupDigits(fileinfo.size()) + " bytes"));
    tableInfo->setItem(row,0,new QTableWidgetItem("Last Modified"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(fileinfo.lastModified().toString("yyyy-MM-dd hh:mm:ss")));
    tableInfo->setItem(row,0,new QTableWidgetItem("Created"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(fileinfo.birthTime().toString("yyyy-MM-dd hh:mm:ss")));
    tableInfo->setItem(row,0,new QTableWidgetItem("Length"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(msToNiceStringExact(song_length_ms,true)));

    tableInfo->setItem(row,0,new QTableWidgetItem("Player Engine"));


    if(SoundManager::getInstance().m_Info1->plugin==0)
    {
        tableInfo->setItem(row++,1,new QTableWidgetItem(PLUGIN_fmod_NAME));
    }
    else
    {
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->pluginName.c_str()));
    }
	
    tableInfo->setItem(row,0,new QTableWidgetItem("Type"));
    tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->fileformat.c_str()));



    if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_game_music_emu)
    {
        tableInfo->setItem(row,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Author"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));
        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setItem(row,0,new QTableWidgetItem("Comments"));
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
            tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
        }

        tableInfo->setItem(row,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->copyright.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Dumper"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->dumper.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("System"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));


    }
    if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_protrekkr)
    {
//        tableInfo->setItem(row,0,new QTableWidgetItem("Title"));
//        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
//        tableInfo->setItem(row,0,new QTableWidgetItem("Author"));
//        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));


    }


    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_libsidplayfp)

    {
        tableInfo->setItem(row,0,new QTableWidgetItem("SID Format"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->fileformatSpecific.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->title.c_str())));
        tableInfo->setItem(row,0,new QTableWidgetItem("Author"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->artist.c_str())));
        tableInfo->setItem(row,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->copyright.c_str())));
        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setItem(row,0,new QTableWidgetItem("Comments"));
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
            tableInfo->setItem(row++,1,new QTableWidgetItem(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str())));
        }

        string sidModel = "Unknown";
        if(SoundManager::getInstance().m_Info1->sidModel==1)
        {
            sidModel = "6581";
        }
        else if(SoundManager::getInstance().m_Info1->sidModel==2)
        {
            sidModel = "8580";
        }
        else if(SoundManager::getInstance().m_Info1->sidModel==3)
        {
            sidModel = "Any";
        }
        string clockSpeed = "Unknown";
        if(SoundManager::getInstance().m_Info1->clockSpeed==1)
        {
            clockSpeed = "PAL";
        }
        else if(SoundManager::getInstance().m_Info1->clockSpeed==2)
        {
            clockSpeed = "NTSC";
        }
        else if(SoundManager::getInstance().m_Info1->clockSpeed==3)
        {
            clockSpeed = "Any";
        }

        tableInfo->setItem(row,0,new QTableWidgetItem("Clock Speed"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(clockSpeed.c_str()));

        string compatibility = "Unknown";
        if(SoundManager::getInstance().m_Info1->compatibility==0)
        {
            compatibility = "C64 compatible";
        }
        else if(SoundManager::getInstance().m_Info1->compatibility==1)
        {
            compatibility = "PSID specific";
        }
        else if(SoundManager::getInstance().m_Info1->compatibility==2)
        {
            compatibility = "Real C64 only";
        }
        else if(SoundManager::getInstance().m_Info1->compatibility==3)
        {
            compatibility = "Requires C64 Basic";
        }

        tableInfo->setItem(row,0,new QTableWidgetItem("Compatibility"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(compatibility.c_str()));

        tableInfo->setItem(row,0,new QTableWidgetItem("Song Speed"));
        tableInfo->setItem(row++,1,new QTableWidgetItem("$" + QString::number(SoundManager::getInstance().m_Info1->songSpeed,16)));

        tableInfo->setItem(row,0,new QTableWidgetItem("SID Model"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(sidModel.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("Load Address"));
        tableInfo->setItem(row++,1,new QTableWidgetItem("$" + QString::number(SoundManager::getInstance().m_Info1->loadAddr,16)));
        tableInfo->setItem(row,0,new QTableWidgetItem("Init Address"));
        tableInfo->setItem(row++,1,new QTableWidgetItem("$" + QString::number(SoundManager::getInstance().m_Info1->initAddr,16)));
        tableInfo->setItem(row,0,new QTableWidgetItem("Play Address"));
        tableInfo->setItem(row++,1,new QTableWidgetItem("$" + QString::number(SoundManager::getInstance().m_Info1->playAddr,16)));
        tableInfo->setItem(row,0,new QTableWidgetItem("Start Subsong"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->startSubSong)));
        tableInfo->setItem(row,0,new QTableWidgetItem("Replayer"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->songPlayer.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("MD5"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->md5New.c_str()));
        tableInfo->setItem(row,0,new QTableWidgetItem("MD5 Old"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->md5Old.c_str()));

    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_vgmplay_legacy)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));

        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Author"));

        tableInfo->setItem(row,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));

        tableInfo->setItem(row,0,new QTableWidgetItem("System"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));

        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Release date"));

        tableInfo->setItem(row,0,new QTableWidgetItem("Dumper"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->dumper.c_str()));

        tableInfo->setItem(row,0,new QTableWidgetItem("Chips used"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString(SoundManager::getInstance().m_Info1->chips.c_str()).left(QString(SoundManager::getInstance().m_Info1->chips.c_str()).length()-2)));

        tableInfo->setItem(row,0,new QTableWidgetItem("Version"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->version >> 8) + "." + QString::number(SoundManager::getInstance().m_Info1->version & 0xFF)));

        tableInfo->setItem(row,0,new QTableWidgetItem("Gain"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->gain,'f', 2 )));

        tableInfo->setItem(row,0,new QTableWidgetItem("Length"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->loopInfo.c_str()));

        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setItem(row,0,new QTableWidgetItem("Comments"));
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
            tableInfo->setItem(row++,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));

        }
    }

    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_wothke_uade_2_13)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numSubsongs)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Subsongs"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->md5New.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("MD5"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Patterns"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->modPatternRows)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Pattern Length"));
    }
	else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_klystron)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Pattern Rows"));

    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_furnace)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("System"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_libfc14audiodecoder)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numUsedPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Used Patterns"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numSndModSeqs)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Snd Mod Seqs"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numVolModSeqs)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Vol Mod Seqs"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_libpac)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Sheets"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numOrders)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Positions"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_adplug)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
            tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
            tableInfo->setItem(row++,0,new QTableWidgetItem("Description"));
        }
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Patterns"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numOrders)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Orders"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_asap)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->author.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Author"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Creation Date"));
        if(SoundManager::getInstance().m_Info1->numChannels==1)
        {
            tableInfo->setItem(row,1,new QTableWidgetItem("Mono"));
            tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        }
        else if(SoundManager::getInstance().m_Info1->numChannels==2)
        {
            tableInfo->setItem(row,1,new QTableWidgetItem("Stereo"));
            tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        }
        string clockSpeed = "Unknown";
        if(SoundManager::getInstance().m_Info1->clockSpeed==1)
        {
            clockSpeed = "PAL";
        }
        else if(SoundManager::getInstance().m_Info1->clockSpeed==0)
        {
            clockSpeed = "NTSC";
        }
        tableInfo->setItem(row,0,new QTableWidgetItem("Clock Speed"));
        tableInfo->setItem(row++,1,new QTableWidgetItem(clockSpeed.c_str()));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_experimental)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Genre"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->copyright.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Year"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Volume"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));

    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_theoretical)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Genre"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->copyright.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Year"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Volume"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));

    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_highly_quixotic)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Genre"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->copyright.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Year"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Volume"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));

    }

    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_vio2sf)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->game.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Game"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->system.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Genre"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->copyright.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Copyright"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Year"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->volumeAmplificationStr.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Volume"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));

    }

    //    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_zxtune)
    //    {
    //        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->author.c_str()));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Author"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->replay.c_str()));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Program"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Patterns"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numOrders)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Orders"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numFrames)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Frames"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->loopFrame)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Loop Frame"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->loopPosition)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Loop Position"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->initialTempo)));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Initial Tempo"));
    //        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
    //        tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));
    //    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_libstsound)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Author"));
        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
            tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
        }
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->songPlayer.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Song Player"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->songType.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Song Type"));

    }

    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_sc68)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->author.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Author"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->composer.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Composer"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->converter.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Converter"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->replay.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Replay"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->hwname.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Hardware"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->rate)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Rate"));
        tableInfo->setItem(row,1,new QTableWidgetItem("$" + QString::number(SoundManager::getInstance().m_Info1->address,16)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Address"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_sndh_player)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->artist.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Artist"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->date.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Year"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->clockSpeed) + " Hz"));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Clock Speed"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->ripper.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Ripper"));
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->converter.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Converter"));
    }
    else if(SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt)
    {
        tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->title.c_str()));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Title"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numChannels)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Channels"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numPatterns)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Patters"));
        tableInfo->setItem(row,1,new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->numOrders)));
        tableInfo->setItem(row++,0,new QTableWidgetItem("Orders"));
        if(!SoundManager::getInstance().m_Info1->comments.empty())
        {
            QPlainTextEdit* plainText = new QPlainTextEdit(QString::fromLocal8Bit(SoundManager::getInstance().m_Info1->comments.c_str()));
            plainText->setReadOnly(true);
            plainText->setEnabled(true);
            tableInfo->setItem(row,1,new QTableWidgetItem(SoundManager::getInstance().m_Info1->comments.c_str()));
            tableInfo->setRowHeight(row,100);
            tableInfo->setCellWidget(row,1,plainText);
            tableInfo->setItem(row++,0,new QTableWidgetItem("Comments"));
        }
    }
    else //use fmod to get the tag info
        {

            FMOD_TAG tag;
            FMOD_RESULT res;
            int numTags = SoundManager::getInstance().getNumTags();

            bool hasWrittenID3V1Header = false;
            bool hasWrittenID3V2Header = false;

            for(int i = 0; i< numTags ; i++)
            {
                res = SoundManager::getInstance().getTag(0,i,&tag);
                if(res==FMOD_OK)
                {
                    if(tag.type==FMOD_TAGTYPE_SHOUTCAST)
                    {
                        if(tag.datatype==FMOD_TAGDATATYPE_STRING)
                        {

                            QString data;
                            data = (char *)tag.data;
                            if(QString(tag.name) == "ARTIST")
                            {
                                tag.name="Artist";
                                playlistItem->info->artist=data.toStdString();
                            }
                            else if(QString(tag.name) == "TITLE")
                            {
                                tag.name="Title";
                                playlistItem->info->title=data.toStdString();
                            }
                            else if(QString(tag.name) == "icy-genre")
                            {
                                tag.name="Genre";
                            }
                            else if(QString(tag.name) == "icy-name")
                            {
                                tag.name="Name";
                            }
                            else if(QString(tag.name) == "icy-url")
                            {
                                tag.name="URL";
                            }
                            else if(QString(tag.name) == "icy-br")
                            {
                                tag.name="Bitrate";
                            }
                            else if(QString(tag.name) == "icy-pub")
                            {
                                tag.name="Published";
                                if(data=="1")
                                {
                                    data="Yes";
                                }
                                else
                                {
                                    data="No";
                                }
                            }
                            if(tag.name!="")
                            {

                                tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                                tableInfo->setItem(row,1,new QTableWidgetItem(data));
                                row++;
                            }
                        }

                    }
                    else if(tag.type==FMOD_TAGTYPE_ID3V1)
                    {
                        if(tag.datatype==FMOD_TAGDATATYPE_STRING)
                        {
                            if(!hasWrittenID3V1Header)
                            {
                                tableInfo->setItem(row,0,new QTableWidgetItem("ID3v1 Tag"));
                                tableInfo->setItem(row,1,new QTableWidgetItem(""));
                                row++;
                                hasWrittenID3V1Header = true;
                            }
                            QString data;
                            QString DataAsString = QString::fromLatin1((char*)tag.data);
                            data = (char *)tag.data;
                            if(QString(tag.name) == "GENRE")
                            {
                                tag.name="Genre";
                                if(data.toInt()>0 && data.toInt()<128)
                                {
                                    DataAsString = ID3V1_GENRES[data.toInt()].c_str();
                                }
                                else
                                {
                                    DataAsString = "";
                                }
                            }
                            else if(QString(tag.name) == "ARTIST")
                            {
                                tag.name="Artist";
                                playlistItem->info->artist=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "TITLE")
                            {
                                tag.name="Title";
                                playlistItem->info->title=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "COMMENT")
                            {
                                tag.name="Comment";
                            }
                            else if(QString(tag.name) == "ALBUM")
                            {
                                tag.name="Album";
                            }
                            else if(QString(tag.name) == "YEAR")
                            {
                                tag.name="Year";
                            }
                            else if(QString(tag.name) == "TRACK")
                            {
                                tag.name="Track";
                            }
                            tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                            tableInfo->setItem(row,1,new QTableWidgetItem(DataAsString));
                            row++;

                        }

                    }
                    else if(tag.type==FMOD_TAGTYPE_ID3V2)
                    {
                        if(tag.datatype==FMOD_TAGDATATYPE_STRING_UTF16)
                        {
                            if(!hasWrittenID3V2Header)
                            {
                                tableInfo->setItem(row,0,new QTableWidgetItem("ID3v2 Tag"));
                                tableInfo->setItem(row,1,new QTableWidgetItem(""));
                                row++;
                                hasWrittenID3V2Header = true;
                            }
                            QString data;
                            QString DataAsString = data.fromUtf16((ushort*)tag.data);
                            if(QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                            {
                                tag.name="Album Title";
                            }
                            else if(QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                            {
                                tag.name="BPM";
                            }
                            else if(QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                            {
                                tag.name="Composer";
                            }
                            else if(QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                            {
                                tag.name="Content type";
                            }
                            else if(QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                            {
                                tag.name="Copyright message";
                            }
                            else if(QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                            {
                                tag.name="Date";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDEN")
                            {
                                tag.name="Encoding time";
                            }
                            else if(QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                            {
                                tag.name="Playlist delay";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDOR")
                            {
                                tag.name="Original release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRC")
                            {
                                tag.name="Recording time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRL")
                            {
                                tag.name="Release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDTG")
                            {
                                tag.name="Tagging time";
                            }
                            else if(QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                            {
                                tag.name="Encoded by";
                            }
                            else if(QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                            {
                                tag.name="Lyricist(s)/Text writer(s)";
                            }
                            else if(QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                            {
                                tag.name="File type";
                            }
                            else if(QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                            {
                                tag.name="Time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TIPL")
                            {
                                tag.name="Involved people list";
                            }
                            else if(QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                            {
                                tag.name="Content group description";
                            }
                            else if(QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                            {
                                tag.name="Title";
                                playlistItem->info->title=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                            {
                                tag.name="Subtitle";
                            }
                            else if(QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                            {
                                tag.name="Initial key";
                            }
                            else if(QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                            {
                                tag.name="Language";
                            }
                            else if(QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                            {
                                tag.name="Length";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMCL")
                            {
                                tag.name="Musician credits list";
                            }
                            else if(QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                            {
                                tag.name="Media type";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMOO")
                            {
                                tag.name="Mood";
                            }
                            else if(QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                            {
                                tag.name="Original album/movie/show title";
                            }
                            else if(QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                            {
                                tag.name="Original filename";
                            }
                            else if(QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                            {
                                tag.name="Original lyricist(s)/text writer(s)";
                            }
                            else if(QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                            {
                                tag.name="Original artist(s)/performer(s)";
                            }
                            else if(QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                            {
                                tag.name="Original Release Year";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TOWN")
                            {
                                tag.name="File owner/licensee";
                            }
                            else if(QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                            {
                                tag.name="Lead performer(s)/Soloist(s)";
                                playlistItem->info->artist=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                            {
                                tag.name="Band/Orchestra/Accompaniment";
                            }
                            else if(QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                            {
                                tag.name="Conductor";
                            }
                            else if(QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                            {
                                tag.name="Interpreted, remixed, or otherwise modified by";
                            }
                            else if(QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                            {
                                tag.name="Part of set";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TPRO")
                            {
                                tag.name="Produced notice";
                            }
                            else if(QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                            {
                                tag.name="Publisher";
                            }
                            else if(QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                            {
                                tag.name="Track number";
                            }
                            else if(QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                            {
                                tag.name="Recording dates";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSN")
                            {
                                tag.name="Internet Radio Station Name";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSO")
                            {
                                tag.name="Internet Radio Station Owner";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOA")
                            {
                                tag.name="Album sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOP")
                            {
                                tag.name="Performer sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOT")
                            {
                                tag.name="Title sort order";
                            }
                            else if(QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                            {
                                tag.name="Size";
                            }
                            else if(QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                            {
                                tag.name="ISRC";
                            }
                            else if(QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                            {
                                tag.name="Encoding Software";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSST")
                            {
                                tag.name="Set subtitle";
                            }
                            else if(QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                            {
                                tag.name="Year";
                            }

                            tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                            tableInfo->setItem(row,1,new QTableWidgetItem(DataAsString));
                            row++;
                        }
                        if(tag.datatype==FMOD_TAGDATATYPE_STRING_UTF8)
                        {
                            if(!hasWrittenID3V2Header)
                            {
                                tableInfo->setItem(row,0,new QTableWidgetItem("ID3v2 Tag"));
                                tableInfo->setItem(row,1,new QTableWidgetItem(""));
                                row++;
                                hasWrittenID3V2Header = true;
                            }
                            QString data;
                            data = (char *)tag.data;
                            QString DataAsString = QString::fromUtf8(data.toStdString().c_str());
                            if(QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                            {
                                tag.name="Album Title";
                            }
                            else if(QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                            {
                                tag.name="BPM";
                            }
                            else if(QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                            {
                                tag.name="Composer";
                            }
                            else if(QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                            {
                                tag.name="Content type";
                            }
                            else if(QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                            {
                                tag.name="Copyright message";
                            }
                            else if(QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                            {
                                tag.name="Date";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDEN")
                            {
                                tag.name="Encoding time";
                            }
                            else if(QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                            {
                                tag.name="Playlist delay";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDOR")
                            {
                                tag.name="Original release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRC")
                            {
                                tag.name="Recording time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRL")
                            {
                                tag.name="Release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDTG")
                            {
                                tag.name="Tagging time";
                            }
                            else if(QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                            {
                                tag.name="Encoded by";
                            }
                            else if(QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                            {
                                tag.name="Lyricist(s)/Text writer(s)";
                            }
                            else if(QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                            {
                                tag.name="File type";
                            }
                            else if(QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                            {
                                tag.name="Time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TIPL")
                            {
                                tag.name="Involved people list";
                            }
                            else if(QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                            {
                                tag.name="Content group description";
                            }
                            else if(QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                            {
                                tag.name="Title";
                                playlistItem->info->title=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                            {
                                tag.name="Subtitle";
                            }
                            else if(QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                            {
                                tag.name="Initial key";
                            }
                            else if(QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                            {
                                tag.name="Language";
                            }
                            else if(QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                            {
                                tag.name="Length";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMCL")
                            {
                                tag.name="Musician credits list";
                            }
                            else if(QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                            {
                                tag.name="Media type";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMOO")
                            {
                                tag.name="Mood";
                            }
                            else if(QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                            {
                                tag.name="Original album/movie/show title";
                            }
                            else if(QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                            {
                                tag.name="Original filename";
                            }
                            else if(QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                            {
                                tag.name="Original lyricist(s)/text writer(s)";
                            }
                            else if(QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                            {
                                tag.name="Original artist(s)/performer(s)";
                            }
                            else if(QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                            {
                                tag.name="Original Release Year";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TOWN")
                            {
                                tag.name="File owner/licensee";
                            }
                            else if(QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                            {
                                tag.name="Lead performer(s)/Soloist(s)";
                                playlistItem->info->artist=DataAsString.toStdString();
                            }
                            else if(QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                            {
                                tag.name="Band/Orchestra/Accompaniment";
                            }
                            else if(QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                            {
                                tag.name="Conductor";
                            }
                            else if(QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                            {
                                tag.name="Interpreted, remixed, or otherwise modified by";
                            }
                            else if(QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                            {
                                tag.name="Part of set";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TPRO")
                            {
                                tag.name="Produced notice";
                            }
                            else if(QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                            {
                                tag.name="Publisher";
                            }
                            else if(QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                            {
                                tag.name="Track number";
                            }
                            else if(QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                            {
                                tag.name="Recording dates";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSN")
                            {
                                tag.name="Internet Radio Station Name";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSO")
                            {
                                tag.name="Internet Radio Station Owner";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOA")
                            {
                                tag.name="Album sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOP")
                            {
                                tag.name="Performer sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOT")
                            {
                                tag.name="Title sort order";
                            }
                            else if(QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                            {
                                tag.name="Size";
                            }
                            else if(QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                            {
                                tag.name="ISRC";
                            }
                            else if(QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                            {
                                tag.name="Encoding Software";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSST")
                            {
                                tag.name="Set subtitle";
                            }
                            else if(QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                            {
                                tag.name="Year";
                            }

                            tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                            tableInfo->setItem(row,1,new QTableWidgetItem(DataAsString));
                            row++;
                        }
                        else if(tag.datatype==FMOD_TAGDATATYPE_STRING)
                        {
                            if(!hasWrittenID3V2Header)
                            {
                                tableInfo->setItem(row,0,new QTableWidgetItem("ID3v2 Tag"));
                                tableInfo->setItem(row,1,new QTableWidgetItem(""));
                                row++;
                                hasWrittenID3V2Header = true;
                            }
                            QString data = QString::fromLatin1((char*)tag.data);
                            tag.name = QString(tag.name).trimmed().toLatin1().data();

                            if(QString(tag.name) == "TALB" || QString(tag.name) == "TAL")
                            {
                                tag.name="Album Title";
                            }
                            else if(QString(tag.name) == "TBPM" || QString(tag.name) == "TBP")
                            {
                                tag.name="BPM";
                            }
                            else if(QString(tag.name) == "TCOM" || QString(tag.name) == "TCM")
                            {
                                tag.name="Composer";
                                playlistItem->info->composer=data.toStdString();
                            }
                            else if(QString(tag.name) == "TCON" || QString(tag.name) == "TCO")
                            {
                                tag.name="Content type";
                            }
                            else if(QString(tag.name) == "TCOP" || QString(tag.name) == "TCR")
                            {
                                tag.name="Copyright message";
                            }
                            else if(QString(tag.name) == "TDAT" || QString(tag.name) == "TDA")
                            {
                                tag.name="Date";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDEN")
                            {
                                tag.name="Encoding time";
                            }
                            else if(QString(tag.name) == "TDLY" || QString(tag.name) == "TDY")
                            {
                                tag.name="Playlist delay";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDOR")
                            {
                                tag.name="Original release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRC")
                            {
                                tag.name="Recording time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDRL")
                            {
                                tag.name="Release time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TDTG")
                            {
                                tag.name="Tagging time";
                            }
                            else if(QString(tag.name) == "TENC" || QString(tag.name) == "TEN")
                            {
                                tag.name="Encoded by";
                            }
                            else if(QString(tag.name) == "TEXT" || QString(tag.name) == "TXT")
                            {
                                tag.name="Lyricist(s)/Text writer(s)";
                            }
                            else if(QString(tag.name) == "TFLT" || QString(tag.name) == "TFT")
                            {
                                tag.name="File type";
                            }
                            else if(QString(tag.name) == "TIME" || QString(tag.name) == "TIM")
                            {
                                tag.name="Time";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TIPL")
                            {
                                tag.name="Involved people list";
                            }
                            else if(QString(tag.name) == "TIT1" || QString(tag.name) == "TT1")
                            {
                                tag.name="Content group description";
                            }
                            else if(QString(tag.name) == "TIT2" || QString(tag.name) == "TT2")
                            {
                                tag.name="Title";
                                playlistItem->info->title=data.toStdString();
                            }
                            else if(QString(tag.name) == "TIT3" || QString(tag.name) == "TT3")
                            {
                                tag.name="Subtitle";
                            }
                            else if(QString(tag.name) == "TKEY" || QString(tag.name) == "TKE ")
                            {
                                tag.name="Initial key";
                            }
                            else if(QString(tag.name) == "TLAN" || QString(tag.name) == "TLA")
                            {
                                tag.name="Language";
                            }
                            else if(QString(tag.name) == "TLEN" || QString(tag.name) == "TLE")
                            {
                                tag.name="Length";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMCL")
                            {
                                tag.name="Musician credits list";
                            }
                            else if(QString(tag.name) == "TMED" || QString(tag.name) == "TMT")
                            {
                                tag.name="Media type";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TMOO")
                            {
                                tag.name="Mood";
                            }
                            else if(QString(tag.name) == "TOAL" || QString(tag.name) == "TOT")
                            {
                                tag.name="Original album/movie/show title";
                            }
                            else if(QString(tag.name) == "TOFN" || QString(tag.name) == "TOF")
                            {
                                tag.name="Original filename";
                            }
                            else if(QString(tag.name) == "TOLY" || QString(tag.name) == "TOL")
                            {
                                tag.name="Original lyricist(s)/text writer(s)";
                            }
                            else if(QString(tag.name) == "TOPE" || QString(tag.name) == "TOA")
                            {
                                tag.name="Original artist(s)/performer(s)";
                                playlistItem->info->artist=data.toStdString();
                            }
                            else if(QString(tag.name) == "TORY" || QString(tag.name) == "TOR")
                            {
                                tag.name="Original Release Year";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TOWN")
                            {
                                tag.name="File owner/licensee";
                            }
                            else if(QString(tag.name) == "TPE1" || QString(tag.name) == "TP1")
                            {
                                tag.name="Lead performer(s)/Soloist(s)";
                                playlistItem->info->artist=data.toStdString();
                            }
                            else if(QString(tag.name) == "TPE2" || QString(tag.name) == "TP2")
                            {
                                tag.name="Band/Orchestra/Accompaniment";
                            }
                            else if(QString(tag.name) == "TPE3" || QString(tag.name) == "TP3")
                            {
                                tag.name="Conductor";
                            }
                            else if(QString(tag.name) == "TPE4" || QString(tag.name) == "TP4")
                            {
                                tag.name="Interpreted, remixed, or otherwise modified by";
                            }
                            else if(QString(tag.name) == "TPOS" || QString(tag.name) == "TPA")
                            {
                                tag.name="Part of set";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TPRO")
                            {
                                tag.name="Produced notice";
                            }
                            else if(QString(tag.name) == "TPUB" || QString(tag.name) == "TPB")
                            {
                                tag.name="Publisher";
                            }
                            else if(QString(tag.name) == "TRCK" || QString(tag.name) == "TRK")
                            {
                                tag.name="Track number";
                            }
                            else if(QString(tag.name) == "TRDA" || QString(tag.name) == "TRD")
                            {
                                tag.name="Recording dates";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSN")
                            {
                                tag.name="Internet Radio Station Name";
                            }
                            //does not exist in v2.0
                            else if(QString(tag.name) == "TRSO")
                            {
                                tag.name="Internet Radio Station Owner";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOA")
                            {
                                tag.name="Album sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOP")
                            {
                                tag.name="Performer sort order";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSOT")
                            {
                                tag.name="Title sort order";
                            }
                            else if(QString(tag.name) == "TSIZ" || QString(tag.name) == "TSI")
                            {
                                tag.name="Size";
                            }
                            else if(QString(tag.name) == "TSRC" || QString(tag.name) == "TRC")
                            {
                                tag.name="ISRC";
                            }
                            else if(QString(tag.name) == "TSSE" || QString(tag.name) == "TSS")
                            {
                                tag.name="Encoding Software";
                            }
                            //id3v2.4
                            else if(QString(tag.name) == "TSST")
                            {
                                tag.name="Set subtitle";
                            }
                            else if(QString(tag.name) == "TYER" || QString(tag.name) == "TYE")
                            {
                                tag.name="Year";
                            }

                            if(data.trimmed().length()>0)
                            {
                                if(QString(tag.name) == "TXXX")
                                {
                                    tableInfo->setItem(row,0,new QTableWidgetItem(data));
                                    char* tst = (char*)tag.data;
                                    QString str;
                                    for(int i = data.length() ; i<tag.datalen;i++)
                                    {
                                        if(tst[i]!=0) //don't add null values to string
                                        {
                                            str+=tst[i];
                                        }
                                    }
                                    tableInfo->setItem(row,1,new QTableWidgetItem(str));
                                    if(data=="replaygain_track_gain")
                                    {
                                        float val = atof(str.trimmed().toStdString().c_str());
                                    }
                                }
                                //iTunes Compilation Flag
                                else if(QString(tag.name) == "TCMP" || QString(tag.name) == "TCP")
                                {
                                    tag.name="Part of a compilation";
                                    if(data=="1" || data.toLower()=="true" || data.toLower()=="yes")
                                    {
                                        data = "Yes";
                                    }
                                    else
                                    {
                                        data = "No";
                                    }
                                    tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                                    tableInfo->setItem(row,1,new QTableWidgetItem(data));
                                }
                                else if(QString(tag.name) == "Content type")
                                {
                                    tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                                    //match "(n)" where n is 0-999
                                    QRegExp rx("[(](\\d{1,3})[)]");
                                    if(rx.exactMatch(data))
                                    {
                                        QStringList list;
                                        list << rx.cap(1);
                                        int number = list[0].toInt();
                                        //check if number is 0-147
                                        if(number>=0 && number<=147)
                                        {
                                            tableInfo->setItem(row,1,new QTableWidgetItem(ID3V1_GENRES[number].c_str()));
                                        }
                                        else
                                        {
                                            tableInfo->setItem(row,1,new QTableWidgetItem(data));
                                        }
                                    }
                                    else if(data=="(RX)")
                                    {
                                        data="Remix";
                                    }
                                    else if(data=="(CR)")
                                    {
                                        data="Cover";
                                    }
                                    else
                                    {

                                        tableInfo->setItem(row,1,new QTableWidgetItem(data));
                                    }


                                }
                                else
                                {
                                    tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                                    tableInfo->setItem(row,1,new QTableWidgetItem(data));
                                }

                                row++;
                            }
                        }
                    }
                    else if(tag.type==FMOD_TAGTYPE_VORBISCOMMENT)
                    {

                        if(tag.datatype==FMOD_TAGDATATYPE_STRING_UTF8)
                        {
                            QString data;
                            data = (char *)tag.data;
                            QString DataAsString = QString::fromUtf8(data.toStdString().c_str());

                            if(QString(tag.name).toUpper() == "ACOUSTID_FINGERPRINT")
                            {
                                tag.name="Acoustic fingerprint";
                            }

                            else if (QString(tag.name).toUpper() == "ACOUSTID_ID")
                            {
                                tag.name="Acoustic ID";
                            }
                            else if (QString(tag.name).toUpper() == "ALBUM")
                            {
                                tag.name="Album";
                            }
                            else if (QString(tag.name).toUpper() == "ALBUM ARTIST")
                            {
                                tag.name="Album Artist";
                            }
                            else if (QString(tag.name).toUpper() == "ALBUMARTIST")
                            {
                                tag.name="Album Artist";
                            }
                            else if (QString(tag.name).toUpper() == "ALBUMARTISTSORT")
                            {
                                tag.name="Album Artist Sort";
                            }
                            else if (QString(tag.name).toUpper() == "ALBUMARTIST_CREDIT")
                            {
                                tag.name="Album Artist Credit";
                            }
                            else if (QString(tag.name).toUpper() == "ARTIST")
                            {
                                tag.name="Artist";
                                playlistItem->info->artist=data.toStdString();
                            }
                            else if (QString(tag.name).toUpper() == "ARTISTSORT")
                            {
                                tag.name="Artist Sort";
                            }
                            else if (QString(tag.name).toUpper() == "ARTIST_CREDIT")
                            {
                                tag.name="Artist Credit";
                            }
                            else if (QString(tag.name).toUpper() == "ASIN")
                            {
                                tag.name="Amazon Standard Identification Number";
                            }
                            else if (QString(tag.name).toUpper() == "BPM")
                            {
                                tag.name="BPM";
                            }
                            else if (QString(tag.name).toUpper() == "CATALOGNUMBER")
                            {
                                tag.name="Catalog number";
                            }
                            else if (QString(tag.name).toUpper() == "COMPILATION")
                            {
                                tag.name="Compilation";
                            }
                            else if (QString(tag.name).toUpper() == "COMPOSER")
                            {
                                tag.name="Composer";
                            }
                            else if (QString(tag.name).toUpper() == "DATE")
                            {
                                tag.name="Date";
                            }
                            else if (QString(tag.name).toUpper() == "DISC")
                            {
                                tag.name="Disc";
                            }
                            else if (QString(tag.name).toUpper() == "DISCC")
                            {
                                tag.name="Discc";
                            }
                            else if (QString(tag.name).toUpper() == "DISCNUMBER")
                            {
                                tag.name="Disc number";
                            }
                            else if (QString(tag.name).toUpper() == "DISCSUBTITLE")
                            {
                                tag.name="Disc subtitle";
                            }
                            else if (QString(tag.name).toUpper() == "DISCTOTAL")
                            {
                                tag.name="Disc total";
                            }
                            else if (QString(tag.name).toUpper() == "ENCODEDBY")
                            {
                                tag.name="Encoded by";
                            }
                            else if (QString(tag.name).toUpper() == "ENCODER")
                            {
                                tag.name="Encoder";
                            }
                            else if (QString(tag.name).toUpper() == "GENRE")
                            {
                                tag.name="Genre";
                            }
                            else if (QString(tag.name).toUpper() == "GROUPING")
                            {
                                tag.name="Grouping";
                            }
                            else if (QString(tag.name).toUpper() == "LABEL")
                            {
                                tag.name="Label";
                            }
                            else if (QString(tag.name).toUpper() == "LANGUAGE")
                            {
                                tag.name="Language";
                            }
                            else if (QString(tag.name).toUpper() == "LYRICS")
                            {
                                tag.name="Lyrics";
                            }
                            else if (QString(tag.name).toUpper() == "MEDIA")
                            {
                                tag.name="Media";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMARTISTID")
                            {
                                tag.name="Musicbrainz Album Artist ID";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMCOMMENT")
                            {
                                tag.name="Musicbrainz Album Comment";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMID")
                            {
                                tag.name="Musicbrainz Album ID";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMSTATUS")
                            {
                                tag.name="Musicbrainz Album Status";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ALBUMTYPE")
                            {
                                tag.name="Musicbrainz Album Type";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_ARTISTID")
                            {
                                tag.name="Musicbrainz Artist ID";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_RELEASEGROUPID")
                            {
                                tag.name="Musicbrainz Release Group ID";
                            }
                            else if (QString(tag.name).toUpper() == "MUSICBRAINZ_TRACKID")
                            {
                                tag.name="Musicbrainz Track ID";
                            }
                            else if (QString(tag.name).toUpper() == "ORIGINALDATE")
                            {
                                tag.name="Original Date";
                            }
                            else if (QString(tag.name).toUpper() == "PUBLISHER")
                            {
                                tag.name="Publisher";
                            }
                            else if (QString(tag.name).toUpper() == "RELEASECOUNTRY")
                            {
                                tag.name="Release Country";
                            }
                            else if (QString(tag.name).toUpper() == "REPLAYGAIN_ALBUM_GAIN")
                            {
                                tag.name="Replaygain Album Gain";
                            }
                            else if (QString(tag.name).toUpper() == "REPLAYGAIN_ALBUM_PEAK")
                            {
                                tag.name="Replaygain Album Peak";
                            }
                            else if (QString(tag.name).toUpper() == "REPLAYGAIN_TRACK_GAIN")
                            {
                                tag.name="Replaygain Track Gain";
                            }
                            else if (QString(tag.name).toUpper() == "REPLAYGAIN_TRACK_PEAK")
                            {
                                tag.name="Replaygain Track Peak";
                            }
                            else if (QString(tag.name).toUpper() == "SCRIPT")
                            {
                                tag.name="Script";
                            }
                            else if (QString(tag.name).toUpper() == "TITLE")
                            {
                                tag.name="Title";
                                playlistItem->info->title=DataAsString.toStdString();
                            }
                            else if (QString(tag.name).toUpper() == "TOTALDISCS")
                            {
                                tag.name="Total Discs";
                            }
                            else if (QString(tag.name).toUpper() == "TOTALTRACKS")
                            {
                                tag.name="Total Tracks";
                            }
                            else if (QString(tag.name).toUpper() == "TRACK")
                            {
                                tag.name="Track";
                            }
                            else if (QString(tag.name).toUpper() == "TRACKC")
                            {
                                tag.name="Total Tracks";
                            }
                            else if (QString(tag.name).toUpper() == "TRACKNUMBER")
                            {
                                tag.name="Track number";
                            }
                            else if (QString(tag.name).toUpper() == "TRACKTOTAL")
                            {
                                tag.name="Total Tracks";
                            }
                            else if (QString(tag.name).toUpper() == "YEAR")
                            {
                                tag.name="Year";
                            }
                            else if (QString(tag.name).toUpper() == "ENCODER")
                            {
                                tag.name="Encoder";
                            }
                            else if (QString(tag.name).toUpper() == "COPYRIGHT")
                            {
                                tag.name="Copyright";
                            }
                            else if (QString(tag.name).toUpper() == "DESCRIPTION")
                            {
                                tag.name="Desciption";
                            }
                            else if (QString(tag.name).toUpper() == "COMMENT")
                            {
                                tag.name="Comment";
                            }
                            else if (QString(tag.name).toUpper() == "DISCID")
                            {
                                tag.name="Disc ID";
                            }
                            tableInfo->setItem(row,0,new QTableWidgetItem(tag.name));
                            tableInfo->setItem(row,1,new QTableWidgetItem(QString::fromUtf8(data.toStdString().c_str())));
                            row++;

                        }

                    }
                }
            }
        }


    tableInfo->setRowCount(row);
    for(int i = 0; i < tableInfo->rowCount(); i++)
    {
        tableInfo->item(i,0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
        tableInfo->item(i,1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter );
    }
    tableInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);


}
