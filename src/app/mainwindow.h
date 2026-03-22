#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DockManager.h>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTcpServer>

#include "channels.h"
#include "filedownloader.h"
#include "fileinfoparser.h"
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

    enum PlayMode
    {
        normal = 0,
        repeatPlaylist = 1,
        repeatSong = 2
    };

    enum LoopPointsState {
        INACTIVE = 0,
        DISABLED = 1,
        UNSET = 2,
        A_SET = 3,
        B_SET = 4
    };

    enum DropWidget
    {
        DROP_IGNORE = 0,
        DROP_TO_PLAYLIST = 1,
        DROP_TO_PLAYLISTS = 2
    };

    bool eventFilter(QObject* obj, QEvent* event);
    explicit MainWindow(int argc, char* argv[], QWidget* parent = nullptr);
    ~MainWindow();
    bool isChannelEnabled(int index) const;
    void setChannelEnabled(int index, bool enable) const;

    static QString getVersion();

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
    void resetShuffle(const QString &);
    bool isShuffleEnabled() const;

    static void swapColumns(QTableView* tableview);

    static QStringList sortPreservingOrder(const QStringList&, const QStringList&);
    void setHvscSonglengthsPath(const QString &);
    QString getHvscSonglengthsPath() const;
    void setBundledHvscSonglengthsUpdateFrequency(const QString &);
    void setBundledHvscSonglengthsPath(const QString &);
    QString getBundledHvscSonglengthsUpdateFrequency() const;
    QString getBundledHvscSonglengthsPath() const;
    qint64 getBundledHvscSonglengthsDownloadEpoch() const;
    void deleteWorkspace(const QString &workspace) const;
    void createNewWorkspace(const QString& filename);
    void setPosition(int offset = 0) const;
    Effect* getEffect() const;
    void resetAll();
    void resetToDefaultColors();
    void changeStyleSheetColor();
    void addDebugText(const QString &) const;
    void restoreFromTray();
    void saveSettings() const;
    void setOutputDevice(int outputDeviceProvided, const QString &);
    void setOutputDeviceSetting(int outputDeviceProvided);
    int getOutputDevice() const;
    void setResetVolume(bool resetVolumeProvided);
    bool getResetVolume() const;
    int getResetVolumeValue() const;
    void setDefaultPlayMode(int defaultPlayModeProvided);
    int getDefaultPlayMode() const;
    int getPlaylistRowHeight() const;
    int getPlaylistsRowHeight() const;
    int getNowPlayingFontSize() const;
    bool isReverbEnabled() const;
    void setReverbPreset(const QString&);
    QString getReverbPreset() const;
    QString getIgnorePrefix() const;
    QString getIgnoreSuffix() const;
    void setIgnorePrefix(const QString &);
    void setIgnoreSuffix(const QString &);
    void setReverbEnabled(bool);
    bool isNormalizeEnabled() const;
    bool getDisplayMilliseconds() const;
    bool getEnqueueItems() const;
    bool getShowCheckBoxLoopPoints() const;
    bool getSystrayChecked() const;
    bool getSystrayOnMinimizeChecked() const;
    bool getMenuBarHiddenChecked() const;
    void setNormalizeEnabled(bool);
    int getNormalizeFadeTime() const;
    void setNormalizeFadeTime(int);
    int getNormalizeThreshold() const;
    void setNormalizeThreshold(int);
    int getNormalizeMaxAmp() const;
    void setNormalizeMaxAmp(int);
    void setResetVolumeValue(int value);
    void setSystrayChecked(bool isChecked);
    void setSystrayOnMinimizeChecked(bool isChecked);
    void setSystrayOnMinimizeEnabled(bool isEnabled);
    void setMenuBarHiddenChecked(bool isChecked);
    void createTrayMenu();
    void updateScrollText() const;

    void setAllowOnlyOneInstanceEnabled(bool enabled);

    void setDisplayMilliseconds(bool enabled);

    void setEnqueueItems(bool enabled);

    void showCheckBoxLoopPoints(bool show);

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
    void setVuMeterPeaksEnabled(bool);
    void setVuMeterPeaksHeight(int);

    bool isVuMeterPeaksEnabled() const;
    int isVuMeterPeaksHeight() const;

    void setPlaylistRowHeight(int);
    void setPlaylistsRowHeight(int);
    void setNowPlayingFontSize(int);

    QString createPlaylist(const QString &);

    void setFmodSeamlessLoopEnabled(bool seamlessLoop);
    bool getFmodSeamlessLoopEnabled() const;

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

    bool isOnlyOneInstanceEnabled() const;
    FileDownloader* filedownloader;
    qint64 bundledHvscSonglengthsDownloadEpoch;

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
    void sendSocketMsg() const;
    void getSocketData();
    void acceptConnection();
    void displayError(QAbstractSocket::SocketError socketError);
    void dockWindowClosed(bool);
    void slot_dockWidgetMenuChecked(ads::CDockWidget*) const;
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

    void fullscreenVisualizer() const;
    void showNextVisualizer() const;
    void fullscreenTracker() const;
    void renamePlaylist();
    void deletePlaylist();
    void deleteAllPlaylists();
    void savePlaylist();
    void savePlaylistAs();
    void deleteFilesInPlaylist();
    void deleteFilesInvertedInPlaylist();
    void showContainingFolder();
    void clearPlaylist();
    void exportInstrumentToWav();
    void exportInstrumentToIff();
    void muteAllChannels() const;
    void unmuteAllChannels() const;

    void selectAllLogWindow() const;
    void copyLogWindow() const;
    void clearLogWindow() const;

    // drag'n'drop
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

    void on_positionSlider_sliderReleased() const;

    void on_playlist_itemDoubleClicked(const QModelIndex&);

    void on_buttonNext_clicked();

    void on_buttonPrev_clicked();

    void on_pushButtonNewPlaylist_clicked();

    void on_listWidget_itemClicked(QListWidgetItem* item);

    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);

    void on_checkBoxShuffle_clicked();

    void on_checkBoxLoop_clicked();

    void on_checkBoxLoopPoints_clicked();

    void on_checkBoxVolumeOn_clicked();

    void on_playlist_doubleClicked(const QModelIndex& index) const;

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

    void on_pitchSlider_valueChanged(int value) const;

    void on_positionSlider_valueChanged(int value) const;

    void on_buttonStop_clicked();

private:
    QString hvscSonglengthsPath;
    QString bundledHvscSonglengthsPath;
    QString bundledHvscSonglengthsUpdateFrequency;
    FileInfoParser* fileInfoParser;
    void loadWorkspaces();
    void updateButtons();
    static QPixmap changeSvgColor(const QString &svgPath, const QColor &color, const QRectF *region = nullptr,
                                  const QColor &color2 = nullptr);

    bool handleMenuBarHiddenEvents(QEvent *event);
    bool altKeyPressCaught = false;

    DropWidget dropWidget;
    static const QString VERSION;
    QTcpServer* tcpServer;
    QTcpSocket* tcpClient;
    QTcpSocket* tcpServerConnection;

    int refreshInfoTimer;

    bool allowOnlyOneInstance;

    QList<ads::CDockWidget*> dockWidgets;
    ads::CDockManager* dockManager;
    ads::CDockWidget* dockWidgetPlayControl;
    ads::CDockWidget* dockWidgetPlaylists;
    ads::CDockWidget* dockWidgetLogMessages;
    ads::CDockWidget* dockWidgetTitle;
    ads::CDockWidget* dockWidgetSamples;
    ads::CDockWidget* dockWidgetChannels;
    ads::CDockWidget* dockWidgetTrackerView;
    ads::CDockWidget* dockWidgetInfo;
    ads::CDockWidget* dockWidgetPlaylist;
    ads::CDockWidget* dockWidgetVisualizer;
    ads::CDockWidget* dockWidgetInstruments;

    VisualizerFullScreen* visualizerFullScreen;
    TrackerFullScreen* trackerFullScreen;
    QMap<QString, QTableView*> tableWidgetPlaylists;

    QString windowTitle;
    QString currentPlaylist;
    QString selectedPlaylist;
    QSystemTrayIcon* tray;

    // tray actions
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

    bool wasMaximized;
    bool vuMeterPeaksEnabled;
    int vuMeterPeaksHeight;

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

    void highlightPlaylistItem(const QString &playlist, int row);
    bool initializeSocket();
    void createMenuWindowTabs();
    void savePlayListSettings() const;
    void checkCommandLine(int argc, char* argv[]);
    void setupIcons();
    void setupAdvancedDockingSystem();
    void savePlayList(QString filename, QString newFilename);
    vector<PlaylistItem*> getPlayListEntriesM3u(QString filename) const;

    void updateSamplesAndInstruments() const;
    void updateInstrumentsAdplug(const Info* info) const;
    void updateInstrumentsAsap(const Info* info) const;
    void updateSamplesFlod(const Info* info) const;
    void updateInstrumentsHivelytracker(const Info* info) const;
    void updateInstrumentsKlystron(const Info* info) const;
    void updateSamplesLibopenmpt(const Info* info) const;
    void updateInstrumentsLibopenmpt(const Info* info) const;
    void updateSamplesLibpac(const Info* info) const;
    void updateSamplesLibxmp(const Info* info) const;
    void updateInstrumentsLibxmp(const Info* info) const;
    void updateSamplesProtrekkr(const Info* info) const;
    void updateInstrumentsProtrekkr(const Info* info) const;
    void updateSamplesUade(const Info* info) const;
    void updateSamplesZxtune(const Info* info) const;

    static QStringList getFilesRecursive(const QString&, const QString& = "", bool = true);
    bool addSong(const QList<QUrl>& urls, int, const QString &, bool);
    void addSong(const QStringList& filenames, int, QString, bool);

    static void addPlaylistEntry(const QTableView* table, int rowPosition, const QString& filename, const QString &fileFormat, const QString &length,
                          int subsong, const QString& fullPath, int lengthInt, const QString& artist);
    void getLength();

    bool loadSound(const QString& fullPath, int subsong);

    void playSongAtRow(int);

    void removeHighlight();
    void createThePopupMenuVisualizer();
    void createThePopupMenuTracker();
    void createThePopupMenuPlaylists();
    void createThePopupLogWindow();
    void createThePopupMenuCurrentPlaylist(const QString &);
    void createThePopupMenuInstruments();
    void createThePopupMenuChannels();
    void exportInstrument(const QString&);
    static void iffWriteChunkHeader(FILE* f, const char* chunkName, uint32_t chunkLen);
    static void iffWriteUint32(FILE* f, uint32_t value);
    static void iffWriteUint16(FILE* f, uint16_t value);
    static void iffWriteUint8(FILE* f, uint8_t value);
    static void iffWriteChunkData(FILE* f, const void* data, size_t length);

    int currentSubsong;
    PlayMode playMode;
    bool buttonNextClicked;

    unsigned int songLengthMs;

    LoopPointsState loopPointsState;
    unsigned int loopPointA;
    unsigned int loopPointB;

    QMap<QString, QVector<unsigned int>> shufflePlayed;
    QMap<QString, QVector<unsigned int>> shuffleToBePlayed;
    QMap<QString, int> currentShufflePosition;

    QString currentPlayingFilepath;
    int currentRow;
    bool isUpdateCurrentRowToNextEnabled;
    QTimer* Timer;
    QTimer* TimerRefreshInfo;

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

    QAction* exportInstrumentWavAction;
    QAction* exportInstrumentIffAction;
    QAction* muteAllChannelsAction;
    QAction* unmuteAllChannelsAction;

    QString lastDir;
    int outputDevice;
    bool resetVolume;
    bool isVolumeMuted;
    int resetVolumeValue;
    int defaultPlayMode;

    bool isSystrayChecked;
    bool isSystrayOnMinimizeChecked;
    bool isSystrayOnMinimizeEnabled;
    bool isMenuBarHiddenChecked;
    bool reverbEnabled;
    QString reverbPreset;
    QString ignoreSuffix;
    QString ignorePrefix;
    bool normalizeEnabled;
    bool displayMilliseconds;
    int normalizeFadeTime;
    int normalizeThreshold;
    int normalizeMaxAmp;

    bool enqueueItems;
    bool isShownCheckBoxLoopPoints;

    bool isFmodSeamlessLoopEnabled;

    int key;
};

#endif // MAINWINDOW_H
