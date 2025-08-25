#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <QDomElement>
#include <string>
#include "DockManager.h"
#include "filedownloader.h"
#include "fileinfoparser.h"
#include "fmod.hpp"
#include "info.h"
#include "playlistitem.h"
#include "channels.h"
#include "qtcpserver.h"
#include "visualizers/trackerfullscreen.h"
#include "visualizers/visualizerfullscreen.h"

using namespace std;

inline QString dataPath;
inline QString libPath;
inline QString userPath;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
#define SWAP16(value) \
    ( \
        (((uint16_t)((value) & 0x00FF)) << 8) | \
        (((uint16_t)((value) & 0xFF00)) >> 8)   \
    )

#define SWAP32(value) \
    ( \
        (((uint32_t)((value) & 0x000000FF)) << 24) | \
        (((uint32_t)((value) & 0x0000FF00)) <<  8) | \
        (((uint32_t)((value) & 0x00FF0000)) >>  8) | \
        (((uint32_t)((value) & 0xFF000000)) >> 24)   \
    )

public:

    enum playmode
    {
        normal = 0,
        repeatPlaylist = 1,
        repeatSong = 2
    };

    enum dropWidget
    {
        DropIgnore = 0,
        DropToPlaylist = 1,
        DropToPlaylists = 2
    };

    bool eventFilter(QObject* obj, QEvent* event);
    explicit MainWindow(int argc, char* argv[], QWidget* parent = nullptr);
    ~MainWindow();
    bool getChannelEnabled(int index);
    void setChannelEnabled(int index, bool enable);
    QString getVersion();

    Ui::MainWindow* ui;


    QByteArray defaultGeometry;
    QByteArray defaultState;
    QByteArray dockingState;

    QAction* workspaceSeparator;
    Channels* channels;

    void setCurrentRow(int);
    int getCurrentRow() const;
    QString getCurrentPlaylist() const;
    QString getSelectedPlaylist() const;
    void resetShuffle(QString);
    bool isShuffleEnabled() const;
    void swapColumns(QTableView* tableview);
    QStringList sortPreservingOrder(const QStringList&, const QStringList&);
    void setHvscSonglengthsFrequency(QString);
    void setHvscSonglengthsPathDownloaded(QString);
    QString getHvscSonglengthsFrequency();
    QString getHvscSonglengthsPathDownloaded();
    qint64 getHvscSonglengthsDownloaded() const;
    void DeleteWorkspace(QString workspace) const;
    void CreateNewWorkspace(const QString& filename);
    void DownloadFile();
    void setPosition(int offset = 0);
    Effect* getEffect() const;
    void resetAll();
    void resetToDefaultColors();
    void changeStyleSheetColor();
    void addDebugText(QString);
    void restoreFromTray();
    void SaveSettings();
    void setOutputDevice(int outputDevice, QString);
    void setOutputDeviceSetting(int outputDevice);
    int getOutputDevice() const;
    void setResetVolume(bool resetVolume);
    bool getResetVolume() const;
    int getResetVolumeValue() const;
    void setDefaultPlaymode(int defaultPlaymode);
    int getDefaultPlaymode() const;
    int getPlaylistRowHeight() const;
    int getPlaylistsRowHeight() const;
    int getNowPlayingFontSize() const;
    bool getReverbEnabled() const;
    void setReverbPreset(const QString&);
    QString getReverbPreset();
    QString getIgnorePrefix();
    QString getIgnoreSuffix();
    void setIgnorePrefix(QString);
    void setIgnoreSuffix(QString);
    void setReverbEnabled(bool);
    bool getNormalizeEnabled() const;
    bool getDisplayMilliseconds() const;
    bool getEnqueueItems() const;
    bool getSystrayOnQuitEnabled() const;
    void setNormalizeEnabled(bool);
    int getNormalizeFadeTime() const;
    void setNormalizeFadeTime(int);
    int getNormalizeThreshold() const;
    void setNormalizeThreshold(int);
    int getNormalizeMaxAmp() const;
    void setNormalizeMaxAmp(int);
    void setResetVolumeValue(int value);
    void setSystrayOnQuitEnabled(bool enabled);
    void createTrayMenu();
    void setScrolltext(QString text);
    void updateScrollText();

    void setAllowOnlyOneInstanceEnabled(bool enabled);

    void setDisplayMilliseconds(bool enabled);

    void setEnqueueItems(bool enabled);

    void setColorMain(QString);
    void setColorMainHover(QString);
    void setColorMedium(QString);
    void setColorBackground(QString);
    void setColorBehindBackground(QString);
    void setColorMainText(QString);
    void setColorSelection(QString);
    void setColorButton(QString);
    void setColorButtonHover(QString);
    void setColorDimmedText(QString);

    void setColorVisualizerTop(const QString&);
    void setColorVisualizerBottom(const QString&);
    void setColorVisualizerMiddle(const QString&);
    void setColorVisualizerPeakColor(const QString&);
    void setColorVisualizerBackground(const QString&);
    void setScrollerReflectionColor(QString);
    void setVUMeterPeaksEnabled(bool);
    void setVUMeterPeaksHeight(int);

    bool getVUMeterPeaksEnabled();
    int getVUMeterPeaksHeight();

    void setPlaylistRowHeight(int);
    void setPlaylistsRowHeight(int);
    void setNowPlayingFontSize(int);

    QString createPlaylist(QString);

    unsigned int getFontSize(QRect, QFont, QString);

    void setFmodSeamlessLoopEnabled(bool seamlessLoop);
    bool getFmodSeamlessLoopEnabled();

    const QString& getColorMain() const;
    const QString& getColorMainHover() const;
    const QString& getColorMedium() const;
    const QString& getColorBackground() const;
    const QString& getColorBehindBackground() const;
    const QString& getColorMainText() const;
    const QString& getColorSelection() const;
    const QString& getColorButton() const;
    const QString& getColorButtonHover() const;
    const QString& getColorDimmedText() const;

    const QString& getColorVisualizerTop() const;
    const QString& getColorVisualizerBottom() const;
    const QString& getColorVisualizerMiddle() const;
    const QString& getColorVisualizerPeakColor() const;
    const QString& getColorVisualizerBackground() const;
    const QString& getScrollerReflectionColor() const;

    bool isOnlyOneInstanceEnabled() const;
    FileDownloader* filedownloader;
    qint64 HvscSonglengthsDownloadedEpoch;

    QMap<QString, QPixmap> icons;


    QString colorMain;
    QString colorMainOld;
    QString colorMainHover;
    QString colorMainHoverOld;
    QString colorMedium;
    QString colorMediumOld;
    QString colorBackground;
    QString colorBackgroundOld;
    QString colorBehindBackground;
    QString colorBehindBackgroundOld;
    QString colorMainText;
    QString colorMainTextOld;
    QString colorSelection;
    QString colorSelectionOld;
    QString colorButton;
    QString colorButtonOld;
    QString colorButtonHover;
    QString colorButtonHoverOld;
    QString colorDimmedText;
    QString colorDimmedTextOld;

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void downloadHvscSonglengthsComplete();
    void sendSocketMsg();
    void getSocketData();
    void acceptConnection();
    void displayError(QAbstractSocket::SocketError socketError);
    void dockWindowClosed(bool);
    void slot_dockWidgetMenuChecked(ads::CDockWidget*);
    void slot_LoadWorkspace(const QString&);
    void quit();
    void muteVolume();
    void clickSysTrayIcon(QSystemTrayIcon::ActivationReason reason);
    void resizeEvent(QResizeEvent*);
    void playNextSong(bool forceNext);
    void playPrevSong();
    void on_buttonPlay_2_clicked();

    void timerProgress();
    void refreshInfo();
    void restoreLayout();
    void openSettings();
    void addFolder();
    void addFiles();

    void fullscreenVisualizer();
    void showNextVisualizer();
    void fullscreenTracker();
    void renamePlaylist();
    void deletePlaylist();
    void deleteAllPlaylists();
    void savePlaylist();
    void savePlaylistAs();
    void deleteFilesInPlaylist();
    void deleteFilesInvertedInPlaylist();
    void showContainingFolder();
    void clearPlaylist();
    void exportInstrumentToWAV();
    void exportInstrumentToIFF();
    void muteAllChannels();
    void unmuteAllChannels() const;

    void selectAllLogWindow() const;
    void copyLogWindow() const;
    void clearLogWindow() const;

    //drag'n'drop
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

    void on_positionSlider_sliderReleased();


    void on_playlist_itemDoubleClicked(const QModelIndex&);


    void on_buttonNext_clicked();

    void on_buttonPrev_clicked();


    void on_pushButtonNewPlaylist_clicked();

    void on_listWidget_itemClicked(QListWidgetItem* item);

    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);

    void on_checkBoxShuffle_clicked();

    void on_checkBoxLoop_clicked();

    void on_checkBoxVolumeOn_clicked();

    void on_playlist_doubleClicked(const QModelIndex& index);

    void on_volumeSlider_valueChanged(int value);

    void on_actionRestore_Default_triggered();

    void on_actionNew_Workspace_triggered();

    void on_actionDelete_Workspace_triggered();

    void on_actionPreferences_triggered();

    void on_actionAdd_folder_triggered();

    void on_actionAdd_file_s_triggered();

    void on_actionAdd_network_stream_triggered();

    void on_actionQuit_triggered();

    void on_actionAbout_BZR_Player_triggered();

    void on_pitchSlider_valueChanged(int value);

    void on_positionSlider_valueChanged(int value);

    void on_buttonStop_clicked();


private:
    QString HvscSonglengthsPathDownloaded;
    QString HvscSonglengthsFrequency;
    FileInfoParser* fileInfoParser;
    void LoadWorkspaces();
    void updateButtons();
    QPixmap ChangeSVGColor(QString filename, QString color);
    void SetAttrRecur(QDomElement elem, QString strtagname, QString strtagname2, QString strattr, QString strattrval);

    dropWidget DropWidget;
    static const string ID3V1_GENRES[];
    static const QString VERSION;
    QTcpServer* tcpServer;
    QTcpSocket* tcpClient;
    QTcpSocket* tcpServerConnection;

    int refreshInfoTimer;


    bool m_bAllowOnlyOneInstanceEnabled;

    QList<ads::CDockWidget*> dockWidgets;
    ads::CDockManager* m_DockManager;
    ads::CDockWidget* DockWidgetPlayControl;
    ads::CDockWidget* DockWidgetPlaylists;
    ads::CDockWidget* DockWidgetLogMessages;
    ads::CDockWidget* DockWidgetTitle;
    ads::CDockWidget* DockWidgetSamples;
    ads::CDockWidget* DockWidgetChannels;
    ads::CDockWidget* DockWidgetTrackerView;
    ads::CDockWidget* DockWidgetInfo;
    ads::CDockWidget* DockWidgetPlaylist;
    ads::CDockWidget* DockWidgetVisualizer;
    ads::CDockWidget* DockWidgetInstruments;

    VisualizerFullScreen* visualizerFullScreen;
    TrackerFullScreen* trackerFullScreen;
    QMap<QString, QTableView*> tableWidgetPlaylists;

    QString windowTitle;
    QString currentPlaylist;
    QString selectedPlaylist;
    QSystemTrayIcon* m_Tray;

    //tray actions
    QAction* nextAction;
    QAction* prevAction;
    QAction* pauseAction;
    QAction* stopAction;
    QAction* playAction;
    QAction* muteAction;
    QAction* quitAction;

    QMenu* trayIconMenu;


    QString colorVisualizerTop;
    QString colorVisualizerBottom;
    QString colorVisualizerMiddle;
    QString colorVisualizerPeak;
    QString colorVisualizerBackground;

    bool wasMaxmimized;
    bool vumeterPeaksEnabled;
    int vumeterPeaksHeight;

    int playlistRowHeight;
    int playlistsRowHeight;
    int nowPlayingFontSize;

    QString colorMainDefault;
    QString colorMainHoverDefault;
    QString colorMediumDefault;
    QString colorBackgroundDefault;
    QString colorBehindBackgroundDefault;
    QString colorMainTextDefault;
    QString colorSelectionDefault;
    QString colorButtonDefault;
    QString colorButtonHoverDefault;
    QString colorDimmedTextDefault;

    int sampleColumnNumberWidth;
    int sampleColumnNameWidth;
    int sampleColumnSizeWidth;
    int sampleColumnLoopStartWidth;
    int sampleColumnLoopEndWidth;
    int sampleColumnVolumeWidth;
    int sampleColumnFinetuneWidth;
    int sampleColumnResolutionWidth;

    int instrumentColumnNumberWidth;
    int instrumentColumnNameWidth;
    int instrumentColumnVolumeWidth;
    int instrumentColumnWaveLengthWidth;

    int infoNameWidth;
    int infoValueWidth;

    void highlightPlaylistItem(QString playlist, int currentRow);
    bool initializeSocket();
    void createMenuWindowTabs();
    void savePlayListSettings();
    void checkCommandLine(int argc, char* argv[]);
    void setupIcons();
    void setupAdvancedDockingSystem();
    void savePlayList(QString filename, QString newFilename);
    vector<PlaylistItem*> getPlayListEntriesM3U(QString filename);
    void updateInstruments();
    QStringList getFilesRecursive(const QString&, const QString& = "", bool = true);
    bool addSong(const QList<QUrl>& urls, int, QString, bool);
    void addSong(const QStringList& filenames, int, QString, bool);

    void addPlaylistEntry(QTableView* table, int rowPosition, QString filename, QString fileFormat, QString length,
                          int subsong, QString fullPath, int lengthInt, QString artist);
    void getLength();
    void loadPlugin(string filename);
    bool loadSound(QString fullPath);

    void PlaySong(int);


    void removeHighlight();
    void createThePopupMenuVisualizer();
    void createThePopupMenuTracker();
    void createThePopupMenuPlaylists();
    void createThePopupLogWindow();
    void createThePopupMenuCurrentPlaylist(QString);
    void createThePopupMenuInstruments();
    void createThePopupMenuChannels();
    void exportInstrument(QString);
    void iffWriteChunkHeader(FILE* f, char* chunkName, uint32_t chunkLen);
    void iffWriteUint32(FILE* f, uint32_t value);
    void iffWriteUint16(FILE* f, uint16_t value);
    void iffWriteUint8(FILE* f, const uint8_t value);
    void iffWriteChunkData(FILE* f, const void* data, size_t length);
    unsigned int getLengthFromSIDDatabase(QString sidfilename, int subsong);

    int currentSubsong;
    playmode Playmode;
    bool buttonNextClicked;


    unsigned int song_length_ms;

    QMap<QString, QVector<unsigned int>> m_ShufflePlayed;
    QMap<QString, QVector<unsigned int>> m_ShuffleToBePlayed;
    QMap<QString, int> m_iCurrentShufflePosition;

    QString currentPlayingFilepath;
    int currentRow;
    bool isUpdateCurrentRowToNextEnabled;
    QTimer* m_Timer;
    QTimer* m_TimerRefreshInfo;

    bool playStarted;
    bool loaded;
    QMenu* popMenu;
    QAction* restoreLayoutAction;
    QAction* openSettingsAction;
    QAction* addFolderAction;
    QAction* addFilesAction;

    QAction* fullscreenVisualizerAction;
    QAction* showNextVisualizerAction;
    QAction* fullscreenTrackerAction;


    QAction* clearPlaylistAction;
    QAction* renamePlaylistAction;
    QAction* deletePlaylistAction;
    QAction* deleteAllPlaylistsAction;
    QAction* savePlaylistAction;
    QAction* savePlaylistAsAction;
    QAction* deleteFilesInPlaylistAction;
    QAction* deleteFilesInPlaylistInvertedAction;
    QAction* showContainingFolderAction;

    QAction* selectAllLogWindowAction;
    QAction* copyLogWindowAction;
    QAction* clearLogWindowAction;


    QAction* exportInstrumentWAVAction;
    QAction* exportInstrumentIFFAction;
    QAction* muteAllChannelsAction;
    QAction* unmuteAllChannelsAction;

    QString lastDir;
    int m_outputDevice;
    bool m_resetVolume;
    bool m_muteVolume;
    int m_resetVolumeValue;
    int m_defaultPlaymode;

    bool m_systrayOnQuitEnabled;
    bool m_reverbEnabled;
    QString m_reverbPreset;
    QString m_ignoreSuffix;
    QString m_ignorePrefix;
    bool m_normalizeEnabled;
    bool m_displayMilliseconds;
    int m_normalizeFadeTime;
    int m_normalizeThreshold;
    int m_normalizeMaxAmp;

    bool m_enqueueItems;

    bool isFmodSeamlessLoopEnabled;

    void ERRCHECK(FMOD_RESULT);


    int key;
    unsigned int version;
};

#endif // MAINWINDOW_H
