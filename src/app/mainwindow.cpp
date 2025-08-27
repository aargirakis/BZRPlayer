#include "mainwindow.h"
#include "dialogdeleteworkspace.h"
#include "dialognewworkspace.h"
#include "filedownloader.h"
#include "myitemdelegate.h"
#include "playlistmodel.h"
#include "qdatetime.h"
#include "qscreen.h"
#include "qsortfilterproxymodel.h"
#include "ui_mainwindow.h"
#include "fmod_errors.h"
#include "info.h"
#include "various.h"
#include <QMimeData>
#include <QFontDatabase>
#include <QFont>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QMenu>
#include <QInputDialog>
#include <QSettings>
#include <QDesktopServices>
#include <QStandardPaths>
#include <cmath>
#include "soundmanager.h"
#include "settingswindow.h"
#include "visualizers/visualizerfullscreen.h"
#include <fstream>
#include <plugins.h>
#include <QTcpServer>
#include <QTcpSocket>
#include "about.h"
#include "DraggableTableView.h"
#include <QHeaderView>

#define NEZPLAYLISTSPLITTER "::<>::?<>"
#define PLAYLISTFIELDSPLITTER "<><>::????"
#define PROJECT_NAME "BZR Player"
#define PROJECT_NAME_VERSIONED PROJECT_NAME " " PROJECT_VERSION
#define PLAYLIST_DEFAULT "Default"
#define PLAYLIST_DEFAULT_EXTENSION ".m3u"
#define PLAYLIST_DEFAULT_FILENAME PLAYLIST_DEFAULT PLAYLIST_DEFAULT_EXTENSION

using namespace std;
const QString MainWindow::VERSION = PROJECT_VERSION;

MainWindow::MainWindow(int argc, char* argv[], QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setlocale(LC_ALL, ".UTF8");

#ifdef WIN32
    const QString exePath = QApplication::applicationDirPath();
    dataPath = exePath + DATA_DIR;
    libPath = exePath + LIB_DIR;
    userPath = exePath + USER_DIR;
#else
#ifdef OUTPUT_DIR
    dataPath = fromUtf8OrLatin1(OUTPUT_DIR) + DATA_DIR;
    libPath = fromUtf8OrLatin1(OUTPUT_DIR) + LIB_DIR;
    userPath = fromUtf8OrLatin1(OUTPUT_DIR) + "/" + USER_DIR;
#else
    dataPath = fromUtf8OrLatin1(DATA_DIR);
    libPath = fromUtf8OrLatin1(LIB_DIR);
    userPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + USER_DIR;
#endif
#endif

    if (!QDir(dataPath).exists()) {
        qFatal("Cannot find directory %s", dataPath.toStdString().c_str());
        QCoreApplication::exit(EXIT_FAILURE);
    }

    bool instanceExists = false;

    QSettings settings(userPath + "/settings.ini",
                       QSettings::IniFormat);
    m_bAllowOnlyOneInstanceEnabled = settings.value("AllowOnlyOneInstance", true).toBool();

    qDebug() << "m_bAllowOnlyOneInstanceEnabled: " << m_bAllowOnlyOneInstanceEnabled;

    if (m_bAllowOnlyOneInstanceEnabled)
    {
        instanceExists = initializeSocket();
    }

    if (instanceExists) return;

    //Fonts needs to be added before the GUI
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "RobotoMono-Regular.ttf");

    ui->setupUi(this);

    setWindowTitle(PROJECT_NAME_VERSIONED);
    windowTitle = PROJECT_NAME_VERSIONED;
    srand(time(NULL));

    setupAdvancedDockingSystem();

    ui->positionSlider->setValue(0);
    ui->labelFilename->setText("");
    ui->centralWidget->setStyleSheet("background:#282828/*background*/;color:#ffffff");


    QFont roboto("Roboto");


    ui->playlist->setFont(roboto);
    ui->labelFilename->setFont(roboto);


    currentRow = 0;
    isUpdateCurrentRowToNextEnabled = true;
    ui->visualizer->init();
    ui->trackerView->init();


    ui->pitchSlider->setSnapToDefault(true);
    ui->pitchSlider->setDefaultValue(100);


    ui->dockWidgetPlaylist->setFloating(false);

    LoadWorkspaces();

    createThePopupMenuPlaylists();


    createThePopupMenuInstruments();
    createThePopupMenuChannels();
    createThePopupLogWindow();

    createTrayMenu();
    m_Tray->show();

    m_outputDevice = settings.value("outputdevice").toInt();
    m_resetVolume = settings.value("resetvolume", false).toBool();
    m_resetVolumeValue = settings.value("resetvolumevalue", 100).toInt();

    lastDir = settings.value("lastOpenedDir", "/").toString();

    m_defaultPlaymode = settings.value("default_playmode", -1).toInt();
    Playmode = static_cast<playmode>(m_defaultPlaymode == -1
                                         ? settings.value("playmode", 0).toInt()
                                         : m_defaultPlaymode);

    if (Playmode == normal) {
        ui->checkBoxLoop->setCheckState(Qt::Unchecked);
    } else if (Playmode == repeatPlaylist) {
        ui->checkBoxLoop->setCheckState(Qt::PartiallyChecked);
    } else {
        ui->checkBoxLoop->setCheckState(Qt::Checked);
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        HvscSonglengthsDownloadedEpoch = settings.value("libsidplayfp/timehvscsonglengthsdownloaded", 0).
                toLongLong();
        HvscSonglengthsPathDownloaded = settings.value("libsidplayfp/hvscsonglengthspath", "").toString();
        HvscSonglengthsFrequency = settings.value("libsidplayfp/updateFrequency", "Weekly").toString();
    }

    ui->checkBoxShuffle->setChecked(settings.value("shuffle", false).toBool());

    m_systrayOnQuitEnabled = settings.value("systrayonquitenabled", false).toBool();

    m_displayMilliseconds = settings.value("displaymilliseconds", false).toBool();

    m_enqueueItems = settings.value("enqueueitems", false).toBool();

    m_normalizeEnabled = settings.value("normalizeenabled", false).toBool();
    m_normalizeFadeTime = settings.value("normalizefadetime", 5000).toInt();
    m_normalizeThreshold = settings.value("normalizethreshold", 10).toInt();
    m_normalizeMaxAmp = settings.value("normalizemaxamp", 20).toInt();
    currentPlaylist = settings.value("currentPlaylist", PLAYLIST_DEFAULT).toString();
    currentRow = settings.value("currentRow", -1).toInt();
    selectedPlaylist = settings.value("selectedPlaylist", PLAYLIST_DEFAULT).toString();
    m_reverbEnabled = settings.value("reverbenabled", false).toBool();
    m_reverbPreset = settings.value("reverbpreset", "Generic").toString();
    m_ignoreSuffix = settings.value("ignoresuffix", "psflib;psf2lib;dsflib").toString();
    m_ignorePrefix = settings.value("ignoreprefix", "").toString();


    colorMainDefault = "#009379/*main*/";
    colorMainHoverDefault = "#00c09e/*main hover*/";
    colorMediumDefault = "#404040/*medium*/";
    colorBackgroundDefault = "#282828/*background*/";
    colorBehindBackgroundDefault = "#161616/*behind-background*/";
    colorMainTextDefault = "#ffffff/*main text*/";
    colorSelectionDefault = "#333333/*selection*/";
    colorButtonDefault = "#b7b7b7/*button*/";
    colorButtonHoverDefault = "#ffffff/*button hover*/";
    colorDimmedTextDefault = "#b1b1b1/*dimmed text*/";

    colorMain = settings.value("colormain", colorMainDefault).toString();
    colorMainHover = settings.value("colormainhover", colorMainHoverDefault).toString();
    colorMedium = settings.value("colormedium", colorMediumDefault).toString();
    colorBackground = settings.value("colorbackground", colorBackgroundDefault).toString();
    colorBehindBackground = settings.value("colorbehindbackground", colorBehindBackgroundDefault).toString();
    colorMainText = settings.value("colormaintext", colorMainTextDefault).toString();
    colorSelection = settings.value("colorselection", colorSelectionDefault).toString();
    colorButton = settings.value("colorbutton", colorButtonDefault).toString();
    colorButtonHover = settings.value("colorbuttonhover", colorButtonHoverDefault).toString();
    colorDimmedText = settings.value("colordimmedtext", colorDimmedTextDefault).toString();

    colorVisualizerTop = settings.value("colorvumetertop", "#ff00dd").toString();
    colorVisualizerMiddle = settings.value("colorvumetermiddle", "#0000ff").toString();
    colorVisualizerBottom = settings.value("colorvumeterbottom", "#00ff5e").toString();
    colorVisualizerBackground = settings.value("colorvisualizerbackground", "#000000").toString();

    sampleColumnNumberWidth = settings.value("samplescolumns/number", 35).toInt();
    sampleColumnNameWidth = settings.value("samplescolumns/name", 185).toInt();
    sampleColumnSizeWidth = settings.value("samplescolumns/size", 70).toInt();
    sampleColumnLoopStartWidth = settings.value("samplescolumns/loopstart", 70).toInt();
    sampleColumnLoopEndWidth = settings.value("samplescolumns/loopend", 70).toInt();
    sampleColumnVolumeWidth = settings.value("samplescolumns/volume", 70).toInt();
    sampleColumnFinetuneWidth = settings.value("samplescolumns/finetune", 70).toInt();
    sampleColumnResolutionWidth = settings.value("samplescolumns/resolution", 70).toInt();

    instrumentColumnNumberWidth = settings.value("instrumentscolumns/number", 35).toInt();
    instrumentColumnNameWidth = settings.value("instrumentscolumns/name", 185).toInt();
    instrumentColumnVolumeWidth = settings.value("instrumentscolumns/volume", 70).toInt();
    instrumentColumnWaveLengthWidth = settings.value("instrumentscolumns/wavelength", 70).toInt();

    infoNameWidth = settings.value("infocolumns/name", 70).toInt();
    infoValueWidth = settings.value("infocolumns/value", 70).toInt();

    colorVisualizerPeak = settings.value("colorvumeterpeak", "#b00e3e").toString();
    vumeterPeaksEnabled = settings.value("vumeterpeaksenabled", true).toBool();
    vumeterPeaksHeight = settings.value("vumeterpeaksheight", 8).toInt();
    getEffect()->setVumeterWidth(settings.value("vumeterwidth", 50.0).toDouble());
    getEffect()->setVumeterOpacity(settings.value("vumeteropacity", 65).toInt());


    getEffect()->setAmplitude(settings.value("scroller/amplitude", 32).toInt());
    getEffect()->setSinusFrequency(settings.value("scroller/frequency", 0.0027).toDouble());
    getEffect()->setSinusSpeed(settings.value("scroller/sinusspeed", 0.1).toDouble());
    getEffect()->setScrollSpeed(settings.value("scroller/scrollspeed", 3).toInt());
    getEffect()->setVerticalScrollPosition(settings.value("scroller/verticalscrollposition", 0).toInt());
    getEffect()->setFontScaleX(settings.value("scroller/fontscaleX", 1).toInt());
    getEffect()->setFontScaleY(settings.value("scroller/fontscaleY", 2).toInt());
    getEffect()->setPrinterFontScaleX(settings.value("printer/fontscaleX", 1).toInt());
    getEffect()->setPrinterFontScaleY(settings.value("printer/fontscaleY", 1).toInt());
    getEffect()->setReflectionEnabled(settings.value("scroller/reflection", true).toBool());
    getEffect()->setStarsEnabled(settings.value("scroller/starfield", true).toBool());
    getEffect()->setCustomScrolltextEnabled(settings.value("scroller/customscrolltextenabled", false).toBool());
    getEffect()->setCustomScrolltext(settings.value("scroller/customscrolltext", "").toString());
    getEffect()->setNumberOfStars(settings.value("scroller/numberofstars", 1000).toInt());
    getEffect()->setStarsDirection(settings.value("scroller/starsdirection", "right").toString());
    getEffect()->setStarSpeed(settings.value("scroller/starspeed", 5).toInt());
    getEffect()->setScrollerEnabled(settings.value("scroller/scrollerenabled", true).toBool());
    getEffect()->setPrinterEnabled(settings.value("scroller/printerenabled", true).toBool());
    getEffect()->setVUMeterEnabled(settings.value("scroller/vumeterenabled", true).toBool());
    getEffect()->setScrollerReflectionColor(
        QColor(settings.value("scroller/reflectionColor", "#00032e").toString()));
    getEffect()->setSinusFontScalingEnabled(settings.value("scroller/setsinusfontscaling", false).toBool());
    getEffect()->setRasterBarsEnabled(settings.value("scroller/rasterbars", false).toBool());
    getEffect()->setNumberOfRasterBars(settings.value("scroller/rasterbarsamount", 8).toInt());
    getEffect()->setRasterBarsSpeed(settings.value("scroller/rasterbarsspeed", 35).toInt());
    getEffect()->setRasterBarsBarHeight(settings.value("scroller/rasterbarsheight", 16).toInt());
    getEffect()->setVumeterOpacity(settings.value("scroller/vumeteropacity", 100).toInt());
    getEffect()->setRasterBarsVerticalSpacing(settings.value("scroller/rasterbarsverticalspacing", 16).toInt());
    getEffect()->setRasterBarsOpacity(settings.value("scroller/rasterbarsopacity", 100).toInt());
    getEffect()->setKeepAspectRatio(settings.value("visualizer/keepaspectratio", false).toBool());
    getEffect()->setResolutionWidth(settings.value("visualizer/resolutionwidth", 320).toInt());
    getEffect()->setResolutionHeight(settings.value("visualizer/resolutionheight", 256).toInt());

    getEffect()->setScrollerFont(settings.value("scroller/font",
                                                dataPath + "/resources/visualizer/bitmapfonts/angels_font.png").
        toString());
    getEffect()->setPrinterFont(settings.value("printer/font",
                                               dataPath + "/resources/visualizer/bitmapfonts/angels_font.png").
        toString());


    getEffect()->setReflectionOpacity(settings.value("reflectionopacity", 50).toInt());

    playlistRowHeight = settings.value("playlistrowheight", "30").toInt();
    playlistsRowHeight = settings.value("playlistsrowheight", "30").toInt();
    nowPlayingFontSize = settings.value("nowplayingfontsize", "16").toInt();


    setColorVisualizerTop(colorVisualizerTop);
    setColorVisualizerBottom(colorVisualizerBottom);
    setColorVisualizerMiddle(colorVisualizerMiddle);
    setColorVisualizerBackground(colorVisualizerBackground);

    setColorVisualizerPeakColor(colorVisualizerPeak);
    setVUMeterPeaksEnabled(vumeterPeaksEnabled);
    setVUMeterPeaksHeight(vumeterPeaksHeight);

    colorMainOld = colorMainDefault;
    colorMainHoverOld = colorMainHoverDefault;
    colorMediumOld = colorMediumDefault;
    colorBackgroundOld = colorBackgroundDefault;
    colorBehindBackgroundOld = colorBehindBackgroundDefault;
    colorMainTextOld = colorMainTextDefault;
    colorSelectionOld = colorSelectionDefault;
    colorButtonOld = colorButtonDefault;
    colorButtonHoverOld = colorButtonHoverDefault;
    colorDimmedTextOld = colorDimmedTextDefault;

    channels = new Channels(this, ui->dockWidgetContents_7);

    QFont font = ui->labelFilename->font();
    font.setPixelSize(16);
    ui->labelFilename->setFont(font);

    ui->tableInfo->setFont(roboto);

    QStringList columnLabelsInfo;
    columnLabelsInfo << "Name" << "Value";
    ui->tableInfo->setColumnCount(columnLabelsInfo.size());
    ui->tableInfo->setHorizontalHeaderLabels(columnLabelsInfo);

    ui->tableInfo->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignRight);
    ui->tableInfo->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->tableInfo->setColumnWidth(0, infoNameWidth);
    ui->tableInfo->setColumnWidth(1, infoValueWidth);

    connect(ui->tableInfo->horizontalHeader(), &QHeaderView::sectionResized,
            [=](int logicalIndex, int oldSize, int newSize) {
                //with lambda

                if (ui->tableInfo->horizontalHeaderItem(logicalIndex) != nullptr) {
                    QString columnText = ui->tableInfo->horizontalHeaderItem(logicalIndex)->text();

                    if (columnText == "Name") {
                        infoNameWidth = newSize;
                    } else if (columnText == "Value") {
                        infoValueWidth = newSize;
                    }
                }
            });


    addDebugText("Settings, reverbpreset: " + m_reverbPreset);
    addDebugText("Settings, m_reverbEnabled: " + QString::number(m_reverbEnabled));

    addDebugText("m_muteVolume: " + QString::number(m_muteVolume));
    addDebugText("m_resetVolume: " + QString::number(m_resetVolume));

    ui->volumeSlider->setDefaultValue(100);

    int vol = settings.value("volume", 100).toInt();
    if (m_resetVolume) {
        ui->volumeSlider->setValue(m_resetVolumeValue);
    } else {
        ui->volumeSlider->setValue(vol);
    }
    m_muteVolume = settings.value("mutevolume", false).toBool();
    if (m_resetVolume) {
        m_muteVolume = false;
    }
    addDebugText("m_muteVolume: " + QString::number(m_muteVolume));
    if (m_muteVolume) {
        ui->checkBoxVolumeOn->setCheckState(Qt::Unchecked);
    } else {
        ui->checkBoxVolumeOn->setCheckState(Qt::Checked);
    }

    SoundManager &sm = SoundManager::getInstance();
    sm.Init(FMOD_OUTPUTTYPE_NOSOUND, ""); //Set sound device to silent


    fileInfoParser = new FileInfoParser();

    refreshInfoTimer = 0;

    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(timerProgress()));


    sm.setNormalizeEnabled(m_normalizeEnabled);
    sm.setNormalizeFadeTime(m_normalizeFadeTime);
    sm.setNormalizeMaxAmp(m_normalizeMaxAmp);
    sm.setNormalizeThreshold(m_normalizeThreshold);
    sm.setReverbPreset(m_reverbPreset);
    sm.setReverbEnabled(m_reverbEnabled);


    m_Timer->start(16);

    qDebug() << "timerProgress started";
    playStarted = false;
    loaded = false;
    currentSubsong = 1;
    buttonNextClicked = true;


    ui->positionSlider->setMaximum(0);

    QDir pathDir(userPath);
    if (!pathDir.exists()) {
        QDir().mkdir(userPath);
    }

    QDir pathDir2(userPath + PLAYLISTS_DIR);
    if (!pathDir2.exists()) {
        QDir().mkdir(userPath + PLAYLISTS_DIR);
    }
    QDir pathDir3(userPath + PLUGINS_DIR);
    if (!pathDir3.exists()) {
        QDir().mkdir(userPath + PLUGINS_DIR);
    }
    QDir pathDir4(userPath + PLUGIN_libsidplayfp_DIR);
    if (!pathDir4.exists()) {
        QDir().mkdir(userPath + PLUGIN_libsidplayfp_DIR);
    }
    QDir pathDir5(userPath + PLUGINS_CONFIG_DIR);
    if (!pathDir5.exists()) {
        QDir().mkdir(userPath + PLUGINS_CONFIG_DIR);
    }


    QDir directory(userPath + PLAYLISTS_DIR);

    QStringList playlists;

    if (!QFileInfo::exists(userPath + PLAYLISTS_DIR + QDir::separator() + PLAYLIST_DEFAULT_FILENAME)) {
        playlists.append(PLAYLIST_DEFAULT_FILENAME);
    }
    //Remove PLAYLIST_DEFAULT_FILENAME and the put it first in playlists
    playlists.append(directory.entryList(QStringList() << "*.m3u" << "*.M3U" << "*.m3u8" << "*.M3U8", QDir::Files));
    playlists.removeOne(PLAYLIST_DEFAULT_FILENAME);
    playlists.insert(0, PLAYLIST_DEFAULT_FILENAME);

    addDebugText("Loading " + QString::number(playlists.count()) + " playlists from " + userPath + PLAYLISTS_DIR);

    changeStyleSheetColor();

    //Read all columns orders, size etc. from settings for all playlists
    settings.beginGroup("playlists");

    QStringList playlistsGeometryKeys = settings.childKeys();
    QMap<QString, QByteArray> playlistsGeometryMap;

    for (const QString& key : playlistsGeometryKeys) {
        QByteArray data = settings.value(key).toByteArray();
        playlistsGeometryMap.insert(key, data);
    }
    settings.endGroup();

    QStringList previousSortedPlaylists = settings.value("playlistOrder").toStringList();
    QStringList sortedPlaylists = sortPreservingOrder(playlists, previousSortedPlaylists);

    foreach(QString filename, sortedPlaylists) {

        QFileInfo f(filename);

		DraggableTableView* tv = new DraggableTableView();
        tv->setDragBackgroundColor(QColor(colorMain.left(7)));
        tv->setDragTextColor(QColor(colorMainText.left(7)));
        tv->setupDelegate(); // has to be called after colors are set

        PlaylistModel* pm = new PlaylistModel(this);
		QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(pm); // create proxy
		proxyModel->setSourceModel(pm);
		tv->setModel(proxyModel);


		tableWidgetPlaylists[f.fileName()] = tv;
		tableWidgetPlaylists[f.fileName()]->setStyleSheet(
			ui->dockWidgetContents_4->styleSheet() +
			"QHeaderView::section{font-family:Roboto;padding:0;} QTableView{padding:9px;}");

		tableWidgetPlaylists[f.fileName()]->setFont(roboto);
		tableWidgetPlaylists[f.fileName()]->installEventFilter(this);

        connect(tableWidgetPlaylists[f.fileName()], SIGNAL(doubleClicked(const QModelIndex &)),
                SLOT(on_playlist_itemDoubleClicked(const QModelIndex &)));

        QStringList columns;
        columns << tr("TITLE") << tr("TYPE") << tr("LENGTH") << tr("SUBSONG");


        //There was no existing default playlist
        if (!QFileInfo::exists(userPath + PLAYLISTS_DIR + QDir::separator() +
                PLAYLIST_DEFAULT_FILENAME) && filename == PLAYLIST_DEFAULT_FILENAME) {
            swapColumns(tableWidgetPlaylists[f.fileName()]);
        }

        tableWidgetPlaylists[f.fileName()]->horizontalHeader()->restoreState(playlistsGeometryMap[f.fileName()]);

        tableWidgetPlaylists[f.fileName()]->horizontalHeader()->setSectionsMovable(true);
        tableWidgetPlaylists[f.fileName()]->horizontalHeader()->setSortIndicatorShown(false);
        tableWidgetPlaylists[f.fileName()]->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        tv->setColumnHidden(4, true);
        tv->setColumnHidden(5, true);
        tv->setColumnHidden(6, true);
        tv->setColumnHidden(7, true);


        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(f.fileName());
        if (f.fileName() == PLAYLIST_DEFAULT_FILENAME) {
            QFont fontItalic = newItem->font();
            fontItalic.setItalic(true);
            newItem->setFont(fontItalic);
        }

        newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
        newItem->setFlags(newItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        ui->listWidget->setDragEnabled(true);
        ui->listWidget->setAcceptDrops(true);
        ui->listWidget->setDropIndicatorShown(true);
        ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
        ui->listWidget->setDefaultDropAction(Qt::MoveAction);
        ui->listWidget->insertItem(ui->listWidget->count(), newItem);
        ui->listWidget->setDragBackgroundColor(QColor(colorMain.left(7)));
        ui->listWidget->setDragTextColor(QColor(colorMainText.left(7)));
        MyItemDelegate* item = new MyItemDelegate(this);
        item->setMainColor(QColor(colorMain.left(7)));
        ui->listWidget->setItemDelegate(item);

        QUrl u = QUrl::fromLocalFile(QDir::separator() + userPath + PLAYLISTS_DIR + "/" + f.fileName());
        QList<QUrl> ql;
        ql.append(u);

        addSong(ql, 0, f.fileName(), false);

        createThePopupMenuCurrentPlaylist(f.fileName());
    }

    if (!QFileInfo::exists(userPath + PLAYLISTS_DIR + QDir::separator() + currentPlaylist)) {
        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
    }
    addDebugText("currentPlaylist: " + currentPlaylist);
    QList<QListWidgetItem *> items = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
    int row = 0;
    for (int i = 0; i < items.count(); i++) {
        row = items.at(i)->listWidget()->row(items.at(i));
        break;
    }

    setPlaylistRowHeight(playlistRowHeight);
    ui->listWidget->setCurrentRow(row);

    on_listWidget_itemClicked(ui->listWidget->currentItem());

    //First save default layout;
    this->setHidden(true);

    QRect BZRrect = geometry();
    BZRrect.setTop(200);
    BZRrect.setLeft(200);
    setGeometry(BZRrect);
    defaultGeometry = saveGeometry();
    this->setHidden(false);
    defaultState = saveState();
    dockingState = m_DockManager->saveState();
    bool shuffle = settings.value("shuffle", false).toBool();
    if (shuffle) {
        resetShuffle(currentPlaylist);
    }


    QUrl imageUrl(PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_URL);
    filedownloader = new FileDownloader(imageUrl, this);

    qint64 currentSeconds = QDateTime::currentDateTime().toSecsSinceEpoch();

    if (PLUGIN_libsidplayfp_LIB != "") {
        if (HvscSonglengthsFrequency == "Never") {
            //Do nothing
        } else if (HvscSonglengthsFrequency == "At every start") {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (HvscSonglengthsFrequency == "Daily" && (currentSeconds - HvscSonglengthsDownloadedEpoch >= 86400)) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (HvscSonglengthsFrequency == "Weekly" && (currentSeconds - HvscSonglengthsDownloadedEpoch >=
                                                            604800)) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (HvscSonglengthsFrequency == "Monthly" && (currentSeconds - HvscSonglengthsDownloadedEpoch >=
                                                             2629743)) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        }
    }

    connect(ui->samples->horizontalHeader(), &QHeaderView::sectionResized,
            [=](int logicalIndex, int oldSize, int newSize) {
                //with lambda

                if (ui->samples->horizontalHeaderItem(logicalIndex) != nullptr) {
                    QString columnText = ui->samples->horizontalHeaderItem(logicalIndex)->text();

                    if (columnText == "#") {
                        sampleColumnNumberWidth = newSize;
                    } else if (columnText == "Name") {
                        sampleColumnNameWidth = newSize;
                    } else if (columnText == "Size") {
                        sampleColumnSizeWidth = newSize;
                    } else if (columnText == "Loopstart") {
                        sampleColumnLoopStartWidth = newSize;
                    } else if (columnText == "Loopend") {
                        sampleColumnLoopEndWidth = newSize;
                    } else if (columnText == "Volume") {
                        sampleColumnVolumeWidth = newSize;
                    } else if (columnText == "Finetune") {
                        sampleColumnFinetuneWidth = newSize;
                    } else if (columnText == "Resolution") {
                        sampleColumnResolutionWidth = newSize;
                    }
                }
            });

    connect(ui->instruments->horizontalHeader(), &QHeaderView::sectionResized,
            [=](int logicalIndex, int oldSize, int newSize) {
                //with lambda

                if (ui->instruments->horizontalHeaderItem(logicalIndex) != nullptr) {
                    QString columnText = ui->instruments->horizontalHeaderItem(logicalIndex)->text();

                    if (columnText == "#") {
                        instrumentColumnNumberWidth = newSize;
                    } else if (columnText == "Name") {
                        instrumentColumnNameWidth = newSize;
                    } else if (columnText == "Volume") {
                        instrumentColumnVolumeWidth = newSize;
                    } else if (columnText == "Wavelen") {
                        instrumentColumnWaveLengthWidth = newSize;
                    }
                }
            });

    ui->dockWidget->hide();
    ui->dockWidgetChannels->hide();
    ui->dockWidgetFilename->hide();
    ui->dockWidgetInfo->hide();
    ui->dockWidgetInstruments->hide();
    ui->dockWidgetLogger->hide();
    ui->dockWidgetPlaylist->hide();
    ui->dockWidgetPlaylists->hide();
    ui->dockWidgetSamples->hide();
    ui->dockWidgetTrackerView->hide();
    ui->dockWidgetVisualizer->hide();
    m_DockManager->restoreState(settings.value("dockingState").toByteArray());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    visualizerFullScreen = new VisualizerFullScreen(ui->visualizer->getEffect());
    trackerFullScreen = new TrackerFullScreen(ui->trackerView->getTracker());
    createThePopupMenuVisualizer();
    createThePopupMenuTracker();
    createMenuWindowTabs();

    ui->labelFilename->installEventFilter(this);
    ui->dockWidgetContents_4->installEventFilter(this);
    visualizerFullScreen->installEventFilter(this);
    trackerFullScreen->installEventFilter(this);
    ui->visualizer->installEventFilter(this);
    ui->trackerView->installEventFilter(this);
    ui->listWidget->installEventFilter(this);
    tableWidgetPlaylists[currentPlaylist]->installEventFilter(this);

    QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0);
    tableWidgetPlaylists[currentPlaylist]->scrollTo(index);
    tableWidgetPlaylists[currentPlaylist]->setCurrentIndex(index);


    QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
    if (l.size() > 0) {
        l.at(0)->setForeground(QColor(colorMain.left(7)));
    }

    setupIcons();

    ui->buttonPrev->installEventFilter(this);
    ui->buttonNext->installEventFilter(this);
    ui->buttonStop->installEventFilter(this);
    ui->buttonPlay_2->installEventFilter(this);
    ui->checkBoxShuffle->installEventFilter(this);
    ui->checkBoxVolumeOn->installEventFilter(this);
    ui->checkBoxLoop->installEventFilter(this);
    ui->pushButtonNewPlaylist->installEventFilter(this);

    ui->Debug->moveCursor(QTextCursor::StartOfLine);

    on_buttonStop_clicked();
    updateButtons();
    highlightPlaylistItem(currentPlaylist, currentRow);
    checkCommandLine(argc, argv);
}

void MainWindow::checkCommandLine(int argc, char* argv[])
{
    QStringList args = qApp->arguments();

    if (args.count() <= 1) return; //we've got no songs/dirs from command line

    args.removeFirst();
    QList<QUrl> urls;
    for (const auto &item: args) {
        urls.append(QUrl().fromLocalFile(item));
    }

    const int rowCountBeforeAddSong = tableWidgetPlaylists[currentPlaylist]->model()->rowCount();

    addSong(urls, 0, PLAYLIST_DEFAULT_FILENAME, false);


    if ((m_bAllowOnlyOneInstanceEnabled && !m_enqueueItems) || !m_bAllowOnlyOneInstanceEnabled) {
        on_listWidget_itemClicked(ui->listWidget->item(0));
        QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
        removeHighlight();
        currentRow = rowCountBeforeAddSong;
        PlaySong(currentRow);
    }
}

bool MainWindow::initializeSocket()
{
    bool instanceExists = false;

    tcpServer = new QTcpServer(this);
    //if we can't listen with server, there is already an instance of the application running
    //so we don't create a server, we create a client instead
    //the client will send command line argumens (filepaths) to the server, when the server has got it,
    //the client will disconnect and close
    if (!tcpServer->listen(QHostAddress::LocalHost, 9860))
    {
        instanceExists = true;
    }
    if (!instanceExists)
    {
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    }
    else
    {
        tcpClient = new QTcpSocket(this);
        connect(tcpClient, SIGNAL(connected()), this, SLOT(sendSocketMsg()));
        connect(tcpClient, SIGNAL(disconnected()), this, SLOT(close()));
        connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this,
                SLOT(displayError(QAbstractSocket::SocketError)));
        connect(tcpClient, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this,
                SLOT(displayError(QAbstractSocket::SocketError)));
        tcpClient->connectToHost(QHostAddress::LocalHost, 9860);
    }

    return instanceExists;
}

void MainWindow::createMenuWindowTabs()
{
    ui->menuWindow->addSeparator();
    foreach(ads::CDockWidget* widg, dockWidgets)
    {
        QAction* action = new QAction(widg->windowTitle());
        if (action->text().toLower().endsWith(".m3u") || action->text().toLower().endsWith(".m3u8"))
        {
            action->setText("Playlist");
        }
        action->setCheckable(true);
        action->setChecked(!widg->isClosed());
        connect(action, &QAction::triggered, this, [&, this, widg] { slot_dockWidgetMenuChecked(widg); });
        connect(widg, SIGNAL(visibilityChanged(bool)), SLOT(dockWindowClosed(bool)));
        ui->menuWindow->addAction(action);
    }
}

void MainWindow::dockWindowClosed(bool b)
{
    foreach(ads::CDockWidget* dock, dockWidgets)
    {
        foreach(QAction *menuAction, ui->menuWindow->actions())
        {
            if (menuAction->text() == dock->windowTitle() || ((dock->windowTitle().toLower().endsWith(".m3u") || dock->
                windowTitle().toLower().endsWith(".m3u8")) && menuAction->text() == "Playlist"))
            {
                menuAction->setChecked(!dock->isClosed());
            }
        }
    }
}

void MainWindow::slot_dockWidgetMenuChecked(ads::CDockWidget* d)
{
    addDebugText("Clicked " + d->windowTitle());
    if (d->isClosed())
    {
        d->toggleView(true);
    }
    else
    {
        d->closeDockWidget();
    }
}

void MainWindow::on_positionSlider_sliderReleased()
{
    setPosition();
}

void MainWindow::setPosition(int offset)
{
    int currentpos = ui->positionSlider->value();
    int maxpos = ui->positionSlider->maximum();
    int pos = currentpos + offset;
    if (pos >= maxpos)
    {
        pos--;
    }
    else if (pos < 0)
    {
        pos = 0;
    }
    SoundManager::getInstance().SetPosition(pos, FMOD_TIMEUNIT_MS);
    addDebugText("Set position to " + QString::number(pos));
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F5)
        {
            ui->buttonPlay_2->click();
        }
        else if (keyEvent->key() == Qt::Key_Right)
        {
            if (QApplication::keyboardModifiers() == (Qt::ShiftModifier))
            {
                setPosition(3000);
            }
            else if (QApplication::keyboardModifiers() == (Qt::ControlModifier))
            {
                setPosition(60000);
            }
            else if (QApplication::keyboardModifiers() == (Qt::ControlModifier | Qt::AltModifier))
            {
                setPosition(300000);
            }
            else
            {
                setPosition(10000);
            }
        }
        else if (keyEvent->key() == Qt::Key_Left)
        {
            if (QApplication::keyboardModifiers() == (Qt::ShiftModifier))
            {
                setPosition(-3000);
            }
            else if (QApplication::keyboardModifiers() == (Qt::ControlModifier))
            {
                setPosition(-60000);
            }
            else if (QApplication::keyboardModifiers() == (Qt::ControlModifier | Qt::AltModifier))
            {
                setPosition(-300000);
            }
            else
            {
                setPosition(-10000);
            }
        }
        if (obj->parent() != nullptr && obj->parent()->objectName() == "Playlist")
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete)
            {
                addDebugText("Deleting playlists items");
                deleteFilesInPlaylist();
            }
            //            else if(keyEvent->key()==Qt::Key_Enter || keyEvent->key()==Qt::Key_Return)
            //            {
            //                int selectedRow = tableWidgetPlaylists[selectedPlaylist]->currentRow();
            //                if(selectedRow>=0)
            //                {
            //                    on_playlist_itemDoubleClicked(tableWidgetPlaylists[selectedPlaylist]->item(selectedRow,0));
            //                }
            //            }
        }
        else if (obj == ui->listWidget)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete)
            {
                addDebugText("Deleting playlist");
                deletePlaylist();
            }
        }
        else if (obj == visualizerFullScreen)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Escape || keyEvent->key() ==
                Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            {
                visualizerFullScreen->hide();
            }
        }

        else if (obj == trackerFullScreen)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Escape || keyEvent->key() ==
                Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            {
                trackerFullScreen->hide();
            }
        }
    }
    else if (event->type() == QEvent::Enter)
    {
        if (obj == ui->buttonPrev)
        {
            ui->buttonPrev->setIcon(icons["prevHover"]);
        }
        else if (obj == ui->buttonNext)
        {
            ui->buttonNext->setIcon(icons["nextHover"]);
        }
        else if (obj == ui->buttonStop)
        {
            ui->buttonStop->setIcon(icons["stopHover"]);
        }
        else if (obj == ui->pushButtonNewPlaylist)
        {
            ui->pushButtonNewPlaylist->setIcon(icons["addHover"]);
        }
        else if (obj == ui->buttonPlay_2)
        {
            if (SoundManager::getInstance().IsPlaying() && !SoundManager::getInstance().GetPaused())
            {
                ui->buttonPlay_2->setIcon(icons["pauseHover"]);
            }
            else
            {
                ui->buttonPlay_2->setIcon(icons["playHover"]);
            }
        }
        else if (obj == ui->checkBoxShuffle)
        {
            if (isShuffleEnabled())
            {
                ui->checkBoxShuffle->setIcon(icons["shuffle-onHover"]);
            }
            else
            {
                ui->checkBoxShuffle->setIcon(icons["shuffle-offHover"]);
            }
        }
        else if (obj == ui->checkBoxVolumeOn)
        {
            if (m_muteVolume)
            {
                ui->checkBoxVolumeOn->setIcon(icons["speaker-offHover"]);
            }
            else
            {
                ui->checkBoxVolumeOn->setIcon(icons["speaker-onHover"]);
            }
        }
        else if (obj == ui->checkBoxLoop)
        {
            if (Playmode == normal)
            {
                ui->checkBoxLoop->setIcon(icons["repeatHover"]);
            }
            else if (Playmode == repeatPlaylist)
            {
                ui->checkBoxLoop->setIcon(icons["repeat-onHover"]);
            }
            else
            {
                ui->checkBoxLoop->setIcon(icons["repeat-1Hover"]);
            }
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        if (obj == ui->buttonPrev)
        {
            ui->buttonPrev->setIcon(icons["prev"]);
        }
        else if (obj == ui->buttonNext)
        {
            ui->buttonNext->setIcon(icons["next"]);
        }
        else if (obj == ui->buttonStop)
        {
            ui->buttonStop->setIcon(icons["stop"]);
        }
        else if (obj == ui->pushButtonNewPlaylist)
        {
            ui->pushButtonNewPlaylist->setIcon(icons["add"]);
        }
        else if (obj == ui->buttonPlay_2)
        {
            if (SoundManager::getInstance().IsPlaying() && !SoundManager::getInstance().GetPaused())
            {
                ui->buttonPlay_2->setIcon(icons["pause"]);
            }
            else
            {
                ui->buttonPlay_2->setIcon(icons["play"]);
            }
        }
        else if (obj == ui->checkBoxShuffle)
        {
            if (isShuffleEnabled())
            {
                ui->checkBoxShuffle->setIcon(icons["shuffle-on"]);
            }
            else
            {
                ui->checkBoxShuffle->setIcon(icons["shuffle-off"]);
            }
        }
        else if (obj == ui->checkBoxVolumeOn)
        {
            if (m_muteVolume)
            {
                ui->checkBoxVolumeOn->setIcon(icons["speaker-off"]);
            }
            else
            {
                ui->checkBoxVolumeOn->setIcon(icons["speaker-on"]);
            }
        }
        else if (obj == ui->checkBoxLoop)
        {
            if (Playmode == normal)
            {
                ui->checkBoxLoop->setIcon(icons["repeat"]);
            }
            else if (Playmode == repeatPlaylist)
            {
                ui->checkBoxLoop->setIcon(icons["repeat-on"]);
            }
            else
            {
                ui->checkBoxLoop->setIcon(icons["repeat-1"]);
            }
        }
    }

    else if (event->type() == QEvent::Wheel && obj == ui->labelFilename && QApplication::keyboardModifiers() == (
        Qt::ControlModifier))
    {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        QPoint numPixels = wheelEvent->pixelDelta();
        QPoint numDegrees = wheelEvent->angleDelta() / 8;
        QFont font = ui->labelFilename->font();
        if (!numPixels.isNull() && numPixels.y() != 0)
        {
            if (font.pixelSize() + numPixels.y() <= 100 && font.pixelSize() + numPixels.y() >= 1)
            {
                setNowPlayingFontSize(font.pixelSize() + numPixels.y());
            }
        }
        else if (!numDegrees.isNull() && numDegrees.y() != 0)
        {
            QPoint numSteps = numDegrees / 15;
            if (font.pixelSize() + (numSteps.y() * 2) <= 100 && font.pixelSize() + (numSteps.y() * 2) >= 1)
            {
                setNowPlayingFontSize(font.pixelSize() + (numSteps.y() * 2));
            }
        }
    }
    else if (event->type() == QEvent::Wheel && obj == ui->visualizer && QApplication::keyboardModifiers() == (
        Qt::ControlModifier))
    {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        QPoint numPixels = wheelEvent->pixelDelta();
        QPoint numDegrees = wheelEvent->angleDelta() / 8;
        float scale = 1;
        if (!numPixels.isNull() && numPixels.y() != 0)
        {
            scale = numPixels.y();
        }
        else if (!numDegrees.isNull() && numDegrees.y() != 0)
        {
            QPoint numSteps = numDegrees / 15;
            scale = numSteps.y() * 2;
        }
        scale = scale / 40;
        scale = 1 - scale;
        if (scale * getEffect()->getResolutionWidth() >= 80 && scale * getEffect()->getResolutionHeight() >= 80 && scale
            * getEffect()->getResolutionWidth() <= 4096 && scale * getEffect()->getResolutionHeight() <= 4096)
        {
            getEffect()->setResolutionWidth(scale * getEffect()->getResolutionWidth());
            getEffect()->setResolutionHeight(scale * getEffect()->getResolutionHeight());
        }
    }
    else if (obj == visualizerFullScreen)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                visualizerFullScreen->hide();
            }
        }
    }

    else if (obj == ui->visualizer)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                visualizerFullScreen->showFullScreen();
            }
        }
    }
    else if (obj == trackerFullScreen)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                trackerFullScreen->hide();
            }
        }
    }
    else if (obj == ui->trackerView)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                trackerFullScreen->showFullScreen();
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

unsigned MainWindow::getFontSize(QRect rect, QFont font, QString text)
{
    int fontSize = 1;

    int i = 0;
    while (i < 30)
    {
        i++;
        QFont f(font);
        f.setPixelSize(fontSize);

        QRect r = QFontMetrics(f).boundingRect(text);
        addDebugText(
            "r.height(): " + QString::number(r.height()) + " rect.height(): " + QString::number(rect.height()) +
            " r.width(): " + QString::number(r.width()) + " rect.width():" + QString::number(rect.width()));
        if (r.height() <= rect.height() && r.width() <= rect.width())
            fontSize++;
        else
            break;
    }
    addDebugText("returning fontisize: " + QString::number(fontSize));
    return fontSize;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
}

void MainWindow::addDebugText(QString debugText)
{
    ui->Debug->appendPlainText(debugText);
}

void MainWindow::refreshInfo()
{
    PlaylistItem pi;
    pi.fullPath = currentPlayingFilepath;
    pi.info = SoundManager::getInstance().m_Info1;
    fileInfoParser->updateFileInfo(ui->tableInfo, &pi);
}

void MainWindow::timerProgress()
{
    unsigned int currentMs = 0;

    if (SoundManager::getInstance().IsPlaying())
    {
        refreshInfoTimer++;
        if (refreshInfoTimer >= 120)
        {
            {
                QFileInfo fileinfo(currentPlayingFilepath);
                if (fileinfo.size() == 0)
                {
                    refreshInfoTimer = 0;
                    refreshInfo();
                }
            }
        }
        if (visualizerFullScreen->isVisible())
        {
            visualizerFullScreen->update();
        }
        else if (trackerFullScreen->isVisible() && !SoundManager::getInstance().GetPaused())
        {
            trackerFullScreen->update();
        }
        else
        {
            ui->visualizer->update();
            if (!SoundManager::getInstance().GetPaused())
            {
                ui->trackerView->update();
            }
        }

        currentMs = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_SUBSONG_MS);
        if (currentMs == 0)
        {
            currentMs = SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MS);;
        }

        if (!ui->positionSlider->isSliderDown())
        {
            ui->labelTimer_2->setText(msToNiceStringExact(currentMs, m_displayMilliseconds));
            ui->positionSlider->setValue(static_cast<int>(currentMs));
        }
    }

    if (playStarted) {
        if (SoundManager::getInstance().m_Info1 != nullptr &&
            (SoundManager::getInstance().m_Info1->isContinuousPlaybackActive ||
             SoundManager::getInstance().m_Info1->isSeamlessLoopActive)) {
            return;
        }

        if (currentMs >= song_length_ms || (!SoundManager::getInstance().IsPlaying() && !SoundManager::getInstance().
            GetPaused()))
        {
            playNextSong(false);
        }
    }
}

void MainWindow::updateButtons()
{
    if (SoundManager::getInstance().IsPlaying() && !SoundManager::getInstance().GetPaused())
    {
        ui->buttonPlay_2->setIcon(icons["pause"]);
    }
    else
    {
        ui->buttonPlay_2->setIcon(icons["play"]);
    }
    if (isShuffleEnabled())
    {
        ui->checkBoxShuffle->setIcon(icons["shuffle-on"]);
    }
    else
    {
        ui->checkBoxShuffle->setIcon(icons["shuffle-off"]);
    }
    if (Playmode == repeatSong)
    {
        ui->checkBoxLoop->setIcon(icons["repeat-1"]);
    }
    else if (Playmode == repeatPlaylist)
    {
        ui->checkBoxLoop->setIcon(icons["repeat-on"]);
    }
    else
    {
        ui->checkBoxLoop->setIcon(icons["repeat"]);
    }
    if (m_muteVolume)
    {
        ui->checkBoxVolumeOn->setIcon(icons["speaker-off"]);
    }
    else
    {
        ui->checkBoxVolumeOn->setIcon(icons["speaker-on"]);
    }
}

MainWindow::~MainWindow()
{
    SaveSettings();
    addDebugText("Quitting....");
    delete ui;
}

void MainWindow::playNextSong(bool forceNext)
{
    addDebugText("Song ended. Time to play next song.");
    addDebugText("Playmode: " + QString::number(Playmode));

    if (!tableWidgetPlaylists.contains(currentPlaylist))
    {
        //This happens if a song is playing and that playlist is deleted while playing
        on_buttonStop_clicked();
        currentRow = 0;
        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
        ui->listWidget->setCurrentRow(0);
        on_listWidget_itemClicked(ui->listWidget->currentItem());
    }
    else if (Playmode == repeatSong && !forceNext)
    {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0 &&
            currentRow != tableWidgetPlaylists[currentPlaylist]->model()->rowCount())
        {
            PlaySong(currentRow);
        }
        else
        {
            on_buttonStop_clicked();
        }
    }
    else if (Playmode == normal || Playmode == repeatPlaylist || forceNext)
    {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() == 0)
        {
            addDebugText("Playlist is empty.");
            on_buttonStop_clicked();
        }
        else
        {
            if (isShuffleEnabled())
            {
                m_iCurrentShufflePosition[currentPlaylist]++;


                if (m_ShuffleToBePlayed[currentPlaylist].size() > 0)
                {
                    if (m_iCurrentShufflePosition[currentPlaylist] >= m_ShufflePlayed[currentPlaylist].size())
                    {
                        unsigned int next = rand() % (m_ShuffleToBePlayed[currentPlaylist].size());
                        addDebugText("Next position shuffled: " + QString::number(next));
                        removeHighlight();
                        currentRow = m_ShuffleToBePlayed[currentPlaylist].at(next);
                        m_ShufflePlayed[currentPlaylist].push_back(currentRow);
                        m_ShuffleToBePlayed[currentPlaylist].remove(next);
                    }
                    else
                    {
                        addDebugText("Previously had this position shuffled.");
                        removeHighlight();
                        currentRow = m_ShufflePlayed[currentPlaylist].at(m_iCurrentShufflePosition[currentPlaylist]);
                    }
                    PlaySong(currentRow);
                }
                else
                {
                    addDebugText("No more songs in playlist.");
                    on_buttonStop_clicked();
                }
            }
            else //Normal or repeat playlist with no shuffle
            {
                if (currentRow < tableWidgetPlaylists[currentPlaylist]->model()->rowCount() - 1)
                {
                    removeHighlight();
                    if (isUpdateCurrentRowToNextEnabled)
                    {
                        currentRow++;
                    }
                    PlaySong(currentRow);
                }
                else
                {
                    if (Playmode == normal || Playmode == repeatSong)
                    {
                        if (isUpdateCurrentRowToNextEnabled)
                        {
                            addDebugText("No more songs in playlist.");
                            on_buttonStop_clicked();
                        }
                        else
                        {
                            PlaySong(currentRow);
                        }
                    }
                    else
                    {
                        addDebugText("Repeat playlist.");
                        removeHighlight();
                        if (isUpdateCurrentRowToNextEnabled)
                        {
                            currentRow = 0;
                        }
                        PlaySong(currentRow);
                    }
                }
            }
        }
    }
    isUpdateCurrentRowToNextEnabled = true;
}

void MainWindow::highlightPlaylistItem(QString playlist, int row)
{
    QModelIndex index = tableWidgetPlaylists[playlist]->model()->index(row, 0, QModelIndex());
    tableWidgetPlaylists[playlist]->model()->setData(index, row, Qt::ForegroundRole);
    tableWidgetPlaylists[currentPlaylist]->update();
    tableWidgetPlaylists[currentPlaylist]->repaint();
}

void MainWindow::resetShuffle(QString playlist)
{
    addDebugText("Reset shuffle for playlist '" + playlist + "'");
    m_ShuffleToBePlayed[playlist].clear();
    m_ShufflePlayed[playlist].clear();

    m_iCurrentShufflePosition[playlist] = 0;

    for (unsigned int e = 0; e < tableWidgetPlaylists[playlist]->model()->rowCount(); e++)
    {
        m_ShuffleToBePlayed[playlist].push_back(e);
    }

    if (tableWidgetPlaylists[playlist]->model()->rowCount() > 0 &&
        currentRow < tableWidgetPlaylists[playlist]->model()->rowCount() - 1)
    {
        m_ShufflePlayed[playlist].push_back(currentRow);
        m_ShuffleToBePlayed[playlist].remove(currentRow);
    }
}
QString MainWindow::getCurrentPlaylist() const
{
    return currentPlaylist;
}
QString MainWindow::getSelectedPlaylist() const
{
    return selectedPlaylist;
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    quit();
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            this->setWindowTitle(windowTitle);
        }
        else
        {
            this->setWindowTitle(PROJECT_NAME_VERSIONED);
        }
        if (isMinimized() == true && m_systrayOnQuitEnabled)
        {
            wasMaxmimized = false;
            hide();
        }
        if (isMaximized() == true && m_systrayOnQuitEnabled)
        {
            wasMaxmimized = true;
        }
    }

    return QMainWindow::changeEvent(event);
}

bool MainWindow::loadSound(QString fullPath)
{
    SoundManager& sm = SoundManager::getInstance();
    addDebugText("Try to load sound (playlistitem): " + fullPath);
    ui->buttonPlay_2->setIcon(icons["pause"]);
    QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0);
    tableWidgetPlaylists[currentPlaylist]->scrollTo(index);
    repaint();

    auto* info = new Info();
    info->clearMemory();
    info->clear();
    info->isPlayModeRepeatSongEnabled = Playmode == repeatSong;
    info->isFmodSeamlessLoopEnabled = getFmodSeamlessLoopEnabled();

    bool loadOK = sm.LoadSound(fullPath, info);

    if (loadOK)
    {
        addDebugText("Loaded sound succesfully.");

        loaded = true;
        currentSubsong = 1;
        currentPlayingFilepath = fullPath;
    }
    else
    {
        currentPlayingFilepath = "";

        addDebugText("Failed to load sound " + fullPath);
        qDebug() << "Failed to load sound setting bool to true";
        cout << "Failed to load sound setting bool to true\n";
        flush(cout);
        QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 6, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index, true, Qt::EditRole);
        tableWidgetPlaylists[currentPlaylist]->update();
    }
    return loadOK;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        //addDebugText("x: " + QString::number(dockWidgetPlaylistRect.left()) + " y: " + dockWidgetPlaylistRect.top() + "w:" + dockWidgetPlaylistRect.width() + "h:" + dockWidgetPlaylistRect.height());
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        const int UNKNOWN_OFFSET = 64;


        if (DockWidgetPlaylists->isCurrentTab())
        {
            QRect dockWidgetPlaylistsRect = DockWidgetPlaylists->geometry();
            QPoint pointPlaylists = DockWidgetPlaylists->mapTo(this, DockWidgetPlaylists->pos());
            dockWidgetPlaylistsRect.setRight(pointPlaylists.x() + dockWidgetPlaylistsRect.width());
            dockWidgetPlaylistsRect.setBottom(
                pointPlaylists.y() + dockWidgetPlaylistsRect.height() - UNKNOWN_OFFSET + 16);
            dockWidgetPlaylistsRect.setLeft(pointPlaylists.x());
            dockWidgetPlaylistsRect.setTop(pointPlaylists.y() - UNKNOWN_OFFSET);
            if (dockWidgetPlaylistsRect.contains(event->pos()))
            {
                event->acceptProposedAction();
                DropWidget = DropToPlaylists;
            }
            else if (DockWidgetPlaylist->isCurrentTab())
            {
                QRect dockWidgetPlaylistRect = DockWidgetPlaylist->geometry();
                QPoint pointPlaylist = DockWidgetPlaylist->mapTo(this, DockWidgetPlaylist->pos());
                dockWidgetPlaylistRect.setRight(pointPlaylist.x() + dockWidgetPlaylistRect.width());
                dockWidgetPlaylistRect.setBottom(
                    pointPlaylist.y() + dockWidgetPlaylistRect.height() - (UNKNOWN_OFFSET / 2) + 8);
                dockWidgetPlaylistRect.setLeft(pointPlaylist.x());
                dockWidgetPlaylistRect.setTop(pointPlaylist.y() - UNKNOWN_OFFSET);
                if (dockWidgetPlaylistRect.contains(event->pos()))
                {
                    event->acceptProposedAction();
                    DropWidget = DropToPlaylist;
                }
                else
                {
                    event->ignore();
                    DropWidget = DropIgnore;
                }
            }
            else
            {
                event->ignore();
                DropWidget = DropIgnore;
            }
        }
        else if (DockWidgetPlaylist->isCurrentTab())
        {
            QRect dockWidgetPlaylistRect = DockWidgetPlaylist->geometry();
            QPoint pointPlaylist = DockWidgetPlaylist->mapTo(this, DockWidgetPlaylist->pos());
            dockWidgetPlaylistRect.setRight(pointPlaylist.x() + dockWidgetPlaylistRect.width());
            dockWidgetPlaylistRect.setBottom(
                pointPlaylist.y() + dockWidgetPlaylistRect.height() - (UNKNOWN_OFFSET / 2) + 8);
            dockWidgetPlaylistRect.setLeft(pointPlaylist.x());
            dockWidgetPlaylistRect.setTop(pointPlaylist.y() - UNKNOWN_OFFSET);
            if (dockWidgetPlaylistRect.contains(event->pos()))
            {
                event->acceptProposedAction();
                DropWidget = DropToPlaylist;
            }
            else if (DockWidgetPlaylists->isCurrentTab())
            {
                QRect dockWidgetPlaylistsRect = DockWidgetPlaylists->geometry();
                QPoint pointPlaylists = DockWidgetPlaylists->mapTo(this, DockWidgetPlaylists->pos());
                dockWidgetPlaylistsRect.setRight(pointPlaylists.x() + dockWidgetPlaylistsRect.width());
                dockWidgetPlaylistsRect.setBottom(
                    pointPlaylists.y() + dockWidgetPlaylistsRect.height() - UNKNOWN_OFFSET + 16);
                dockWidgetPlaylistsRect.setLeft(pointPlaylists.x());
                dockWidgetPlaylistsRect.setTop(pointPlaylists.y() - UNKNOWN_OFFSET);
                if (dockWidgetPlaylistsRect.contains(event->pos()))
                {
                    event->acceptProposedAction();
                    DropWidget = DropToPlaylists;
                }
                else
                {
                    event->ignore();
                    DropWidget = DropIgnore;
                }
            }
            else
            {
                event->ignore();
                DropWidget = DropIgnore;
            }
        }
        else
        {
            event->ignore();
            DropWidget = DropIgnore;
        }
    }
    else
    {
        event->ignore();
        DropWidget = DropIgnore;
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (DropWidget == DropToPlaylist)
    {
        QList<QUrl> list = event->mimeData()->urls();
        addSong(list, 0, ui->listWidget->currentItem()->text(), false);
    }
    else if (DropWidget == DropToPlaylists)
    {
        QList<QUrl> list = event->mimeData()->urls();
        addSong(list, 0, ui->listWidget->currentItem()->text(), true);
    }
}

/*!
 gets all files in the given directory recursively as a QStringList
 */
QStringList MainWindow::getFilesRecursive(const QString& dirName, const QString& extension, bool clearStaticVar)
{
    static QStringList list;
    if (clearStaticVar)
    {
        list.clear();
    }
    QDir directory(dirName);
    if (extension != "")
    {
        QStringList filters;
        filters << "*." + extension;
        directory.setNameFilters(filters);
    }
    else
    {
        directory.setFilter(QDir::AllEntries);
    }
    QStringList dirs = directory.entryList(QDir::Dirs | QDir::System | QDir::Hidden);
    QStringList files = directory.entryList(QDir::Files | QDir::System | QDir::Hidden);
    int i, k = 0;
    for (i = 0; i < dirs.count(); i++)
    {
        if (dirs[i] == "." || dirs[i] == "..") continue;
        getFilesRecursive(directory.path() + '/' + dirs[i], extension, false);
    }
    for (k = 0; k < files.count(); k++)
    {
        list.append(directory.path() + '/' + files[k]);
    }

    return list;
}

/*!
 Takes a Qlist of QUrl:s and converts them to a QStringList
 This QStringList is then passed on to addSong(QStringList)
 Returns false if an empty dir was drag'n'dropped, otherwise true
*/
bool MainWindow::addSong(const QList<QUrl>& urls, int row, QString playlistName, bool createNewPlaylist)
{
    QListIterator<QUrl> it(urls);
    QStringList stringList;
    while (it.hasNext())
    {
        QUrl url = it.next();
        addDebugText("opening url: " + url.toString());
        if (url.isLocalFile())
        {
            QFile file(url.toLocalFile());
            QFileInfo fileinfo(file);
            if (!fileinfo.isDir())
            {
                stringList.append(fileinfo.filePath());
            }
            else
            {
                //it's a dir, get all files recursively
                QStringList stringListNew = getFilesRecursive(fileinfo.filePath());
                for (int i = 0; i < stringListNew.size(); i++)
                {
                    stringList.append(stringListNew.at(i));
                }
            }
        }
        else //it's a (one) url
        {
            stringList.append(url.toString());
        }
    }
    //this may happen if drag'n'drop an empty dir
    if (stringList.size() > 0)
    {
        addSong(stringList, row, playlistName, createNewPlaylist);
        return true;
    }
    return false;
}

/*!
 Takes a QStringList of filenames, rownumber and a playlist as input.
 Adds all filenames to that playlist, inserting it into that row.
 If createNewPlaylist is true, creates a new playlist with that playlist name
 previously specified.
*/
void MainWindow::addSong(const QStringList& filenames, int row, QString playlistName, bool createNewPlaylist)
{
    QListIterator<QString> it(filenames);
    while (it.hasNext())
    {
        QString filenameFullPath = it.next();
        QFile file(filenameFullPath);
        QFileInfo fileInfo(file.fileName());
        QString filename(fileInfo.fileName());

        QStringList ignorePrefixList = getIgnorePrefix().split(";");
        QStringListIterator itIgnorePrefixFiles(ignorePrefixList);
        QStringList ignoreSuffixList = getIgnoreSuffix().split(";");
        QStringListIterator itIgnoreSuffixFiles(ignoreSuffixList);
        bool ignoreThisFile = false;
        while (itIgnorePrefixFiles.hasNext())
        {
            QString ignoreFilePrefix = itIgnorePrefixFiles.next() + ".";
            if (filename.startsWith(ignoreFilePrefix, Qt::CaseInsensitive))
            {
                ignoreThisFile = true;
            }
        }

        if (ignoreThisFile)
        {
            continue;
        }

        while (itIgnoreSuffixFiles.hasNext())
        {
            QString ignoreFileSuffix = "." + itIgnoreSuffixFiles.next();
            if (filename.endsWith(ignoreFileSuffix, Qt::CaseInsensitive))
            {
                ignoreThisFile = true;
            }
        }

        if (ignoreThisFile)
        {
            continue;
        }

        addDebugText("Added " + filenameFullPath);
        if (filenameFullPath.endsWith(".m3u", Qt::CaseInsensitive) || filenameFullPath.endsWith(
            ".m3u8", Qt::CaseInsensitive))
        {
            if (createNewPlaylist)
            {
                //Create a new playlist with the filename and add all those files to that playlist
                playlistName = createPlaylist(filename);
            }

            vector<PlaylistItem*> playlistEntries = getPlayListEntriesM3U(filenameFullPath);

            for (unsigned int e = 0; e < playlistEntries.size(); e++)
            {
                QString length = "";

                if (playlistEntries.at(e)->length > 0)
                {
                    length = msToNiceStringExact(playlistEntries.at(e)->length, m_displayMilliseconds);
                }
                QTableWidgetItem* timeItem = new QTableWidgetItem(length);
                QString subsong = "";
                if (playlistEntries.at(e)->startSubsongPlayList != -1)
                {
                    subsong = QString::number(playlistEntries.at(e)->startSubsongPlayList);
                }
                QTableWidgetItem* subsongItem = new QTableWidgetItem(subsong);

                addPlaylistEntry(tableWidgetPlaylists[playlistName],
                                 tableWidgetPlaylists[playlistName]->model()->rowCount(), playlistEntries.at(e)->title,
                                 playlistEntries.at(e)->fileFormat, length, playlistEntries.at(e)->startSubsongPlayList,
                                 playlistEntries.at(e)->fullPath, playlistEntries.at(e)->length,
                                 playlistEntries.at(e)->artist);


                //playlists[playlistName].append(playlistEntries.at(e));


                //addDebugText("Added from m3u: " + QUrl::fromPercentEncoding(playlistEntries[e]->fullPath.toStdString().c_str()));
            }
        }
        else
        {
            addPlaylistEntry(tableWidgetPlaylists[playlistName],
                             tableWidgetPlaylists[playlistName]->model()->rowCount(), filename, "", 0, 0,
                             filenameFullPath, 0, "");
            //playlists[playlistName].append(new PlaylistItem(SoundManager::getInstance().m_Info1,filenameFullPath,0));
        }
    }
}

void MainWindow::addPlaylistEntry(QTableView* table, int rowPosition, QString filename, QString fileFormat,
                                  QString length, int subsong, QString fullPath, int lengthInt, QString artist)
{
    table->model()->insertRows(rowPosition, 1, QModelIndex());


    QString subsongStr = QString::number(subsong);
    if (subsongStr == "0")
    {
        subsongStr = "";
    }
    QModelIndex index = table->model()->index(rowPosition, 0, QModelIndex());
    table->model()->setData(index, filename, Qt::EditRole);
    index = table->model()->index(rowPosition, 1, QModelIndex());
    table->model()->setData(index, fileFormat, Qt::EditRole);
    index = table->model()->index(rowPosition, 2, QModelIndex());
    table->model()->setData(index, length, Qt::EditRole);
    index = table->model()->index(rowPosition, 3, QModelIndex());
    table->model()->setData(index, subsongStr, Qt::EditRole);
    index = table->model()->index(rowPosition, 4, QModelIndex());
    table->model()->setData(index, fullPath, Qt::EditRole);
    index = table->model()->index(rowPosition, 5, QModelIndex());
    table->model()->setData(index, lengthInt, Qt::EditRole);
    index = table->model()->index(rowPosition, 8, QModelIndex());
    table->model()->setData(index, artist, Qt::EditRole);
}

void MainWindow::getLength()
{
    if (SoundManager::getInstance().IsPlaying())
    {
        //check if this sound has subsongs

        unsigned int subsongs = 1;
        subsongs = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_SUBSONG);
        if (subsongs == 0)
        {
            subsongs = 1;
        }
        SoundManager::getInstance().m_Info1->numSubsongs = subsongs;
        addDebugText("subsongs: " + QString::number(subsongs));
        SoundManager::getInstance().m_Info1->currentSubsong = currentSubsong;
        song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_MS);
        addDebugText("song_length_ms: " + QString::number(song_length_ms));
        if (song_length_ms == 0 || song_length_ms == -1)
        {
            song_length_ms = SoundManager::getInstance().GetLength(FMOD_TIMEUNIT_SUBSONG_MS);
            addDebugText("song_length_ms: " + QString::number(song_length_ms));
        }


        if (song_length_ms == 0)
        {
            song_length_ms = -1;
        }
        //        if(song_length_ms==-1 && playlists[currentPlaylist].at(currentRow)->length>0)
        //        {
        //            song_length_ms=playlists[currentPlaylist].at(currentRow)->length;
        //        }
        if (song_length_ms == -1)
        {
            ui->positionSlider->setMaximum(0);
        }
        else
        {
            ui->positionSlider->setMaximum(song_length_ms);
        }

        ui->labelFileLength_2->setText(msToNiceStringExact(song_length_ms, m_displayMilliseconds));
    }
    else
    {
        ui->positionSlider->setMaximum(0);
        if (m_displayMilliseconds)
        {
            ui->labelFileLength_2->setText("0:00.000");
        }
        else
        {
            ui->labelFileLength_2->setText("0:00");
        }
    }
}


void MainWindow::on_buttonPlay_2_clicked()
{
    if (loaded)
    {
        if (SoundManager::getInstance().IsPlaying() && !SoundManager::getInstance().GetPaused())
        {
            playAction->setText("Play");
            SoundManager::getInstance().Pause(true);
        }
        else
        {
            playAction->setText("Pause");
            SoundManager::getInstance().Pause(false);
        }
    }
    else
    {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0)
        {
            if (currentRow < 0)
            {
                removeHighlight();
                currentRow++;
            }
            PlaySong(currentRow);
            playAction->setText("Pause");
        }
        else
        {
            addFiles();
            if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0)
            {
                PlaySong(currentRow);
                playAction->setText("Pause");
            }
        }
    }
    updateButtons();
    isUpdateCurrentRowToNextEnabled = true;
}


void MainWindow::resetToDefaultColors()
{
    colorMainOld = colorMain;
    colorMainHoverOld = colorMainHover;
    colorMediumOld = colorMedium;
    colorBackgroundOld = colorBackground;
    colorBehindBackgroundOld = colorBehindBackground;
    colorMainTextOld = colorMainText;
    colorSelectionOld = colorSelection;
    colorButtonOld = colorButton;
    colorButtonHoverOld = colorButtonHover;
    colorDimmedTextOld = colorDimmedText;

    colorMain = colorMainDefault;
    colorMainHover = colorMainHoverDefault;
    colorMedium = colorMediumDefault;
    colorBackground = colorBackgroundDefault;
    colorBehindBackground = colorBehindBackgroundDefault;
    colorMainText = colorMainTextDefault;
    colorSelection = colorSelectionDefault;
    colorButton = colorButtonDefault;
    colorButtonHover = colorButtonHoverDefault;
    colorDimmedText = colorDimmedTextDefault;

    changeStyleSheetColor();
}

void MainWindow::on_playlist_itemDoubleClicked(const QModelIndex& index)
{
    addDebugText("dbl click! " + index.model()->data(index).toString());
    addDebugText("dbl click! " + QString::number(index.row()));

    //Remove highlighted playlist
    //There might by a state where no playlist is highlighted if a user
    //Removed a plying playlist
    QList<QListWidgetItem*> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
    if (l.size() > 0)
    {
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
    }

    removeHighlight();
    currentPlaylist = ui->listWidget->currentItem()->text();
    if (isShuffleEnabled())
    {
        resetShuffle(currentPlaylist);
    }

    removeHighlight();
    currentRow = index.row();

    PlaySong(currentRow);
    isUpdateCurrentRowToNextEnabled = true;
}

void MainWindow::updateScrollText()
{
    if (!loaded) return;

    if (ui->visualizer->getEffect()->getCustomScrolltextEnabled()) {
        //TODO this will only be blank after program start
        ui->visualizer->getEffect()->setScrollText(ui->visualizer->getEffect()->getCustomScrolltext());
    } else {
        QString visualizerText = "";
        visualizerText = fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->artist);
        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->game);
        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->comments);
        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->copyright);
        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->date);
        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        if (SoundManager::getInstance().m_Info1->instruments != nullptr) {
            for (int i = 0; i < SoundManager::getInstance().m_Info1->numInstruments; i++) {
                visualizerText += fromUtf8OrLatin1(
                    SoundManager::getInstance().m_Info1->instruments[i]) + QString(" ");
            }
        }
        if (SoundManager::getInstance().m_Info1->samples != nullptr) {
            for (int i = 0; i < SoundManager::getInstance().m_Info1->numSamples; i++) {
                visualizerText +=fromUtf8OrLatin1(SoundManager::getInstance().m_Info1->samples[i]) +
                        QString(" ");
            }
        }
        ui->visualizer->getEffect()->setScrollText(visualizerText);
    }
}

void MainWindow::PlaySong(int currentRow)
{
    QString fullPath = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 4).data().toString();
    addDebugText("Now playing and loading sound " + fullPath);


    QString subsong = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 3).data().toString();

    if (subsong != "")
    {
        subsong = "[" + subsong + "]";
    }
    QFileInfo fileInfo(fullPath);
    QString suffix(fileInfo.suffix());
    QString baseName(fileInfo.baseName());


    setOutputDevice(m_outputDevice, baseName + subsong + "." + suffix);

    addDebugText("setOutputDevice with extra data: " + fullPath + "/" + baseName + subsong + "." + suffix);
    playStarted = true;
    if (loadSound(fullPath))
    {
        addDebugText("Check if we are playing already.");
        if (SoundManager::getInstance().IsPlaying())
        {
            addDebugText("Stopping sound.");
            playAction->setText("Play");
            SoundManager::getInstance().Stop();
        }

        addDebugText("Now playing");


        SoundManager::getInstance().PlayAudio(true);

        int vol = ui->volumeSlider->value();
        SoundManager::getInstance().SetVolume((float)vol / 100);

        /* TODO:
         *  this workaround calculates the maximum pitch slider value (see ticket #623)
         *  seems fmod can play at freqs slightly greater than 749700 (we should find the highest possible one)
         */
        float freq = SoundManager::getInstance().GetNominalFrequency();
        int pitchSliderMaxValue = 749700 / freq * 100;
        ui->pitchSlider->setMaximum(pitchSliderMaxValue);
        SoundManager::getInstance().SetFrequencyByMultiplier(ui->pitchSlider->value() / 100.0);

        addDebugText("Now after playing");
        //        addDebugText("startSubsongPlayList: " + QString::number(playlists[currentPlaylist].at(playlistNumber)->startSubsongPlayList));
        currentSubsong = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 3).data().toInt();
        if (currentSubsong < 1)
        {
            currentSubsong = 1;
        }

        SoundManager::getInstance().SetPosition(currentSubsong - 1, FMOD_TIMEUNIT_SUBSONG);

        /*
         * FMOD Channel SetPosition callback with timeunit FMOD_TIMEUNIT_MS and position 0
         * is already internally invoked by FMOD System CreateSound
         * (along with other callbacks, including the read one for the pre-buffering):
         * however here it must be called again after having been set the subsong
         */
        SoundManager::getInstance().SetPosition(0, FMOD_TIMEUNIT_MS);

        addDebugText("Mute is:" + QString::number(m_muteVolume));
        SoundManager::getInstance().SetMute(m_muteVolume);
        SoundManager::getInstance().Pause(false);


        channels->updateChannels();

        getLength();

        ui->visualizer->init();
        ui->trackerView->init();

        PlaylistItem pi;
        pi.fullPath = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 4).data().toString();
        pi.info = SoundManager::getInstance().m_Info1;

        fileInfoParser->updateFileInfo(ui->tableInfo, &pi);


        QFile file(pi.fullPath);
        QFileInfo fileInfo(file.fileName());
        QString filename(fileInfo.fileName());
        QString artist = "";
        if (pi.info->title != "")
        {
            if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_fmod)
            {
                filename = pi.info->title.c_str();
            }
            else
            {
                filename = fromUtf8OrLatin1(pi.info->title);
            }
        }

        if (pi.info->artist != "")
        {
            if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_fmod)
            {
                artist = pi.info->artist.c_str();
            }
            else
            {
                artist = fromUtf8OrLatin1(pi.info->artist);
            }
        }
        else if (pi.info->author != "")
        {
            if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_fmod)
            {
                artist = pi.info->author.c_str();
            }
            else
            {
                artist = fromUtf8OrLatin1(pi.info->author);
            }
        }
        else if (pi.info->composer != "")
        {
            if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_fmod)
            {
                artist = pi.info->composer.c_str();
            }
            else
            {
                artist = fromUtf8OrLatin1(pi.info->composer);
            }
        }
        if (artist != "")
        {
            ui->labelFilename->setText(artist + " - " + filename);
            windowTitle = artist + " - " + filename + " - " + PROJECT_NAME;
        }
        else
        {
            ui->labelFilename->setText(filename);
            windowTitle = filename + " - " + PROJECT_NAME;
        }

        if (isMinimized() || !this->isVisible())
        {
            this->setWindowTitle(windowTitle);
        }
        else
        {
            this->setWindowTitle(PROJECT_NAME_VERSIONED);
        }

        if (m_Tray->isVisible())
        {
            m_Tray->setToolTip(windowTitle);
        }

        QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0, QModelIndex());
        if (pi.info->title != "")
        {
            tableWidgetPlaylists[currentPlaylist]->model()->setData(index, fromUtf8OrLatin1(pi.info->title), Qt::EditRole);
        }


        index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 1, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(
            index, SoundManager::getInstance().m_Info1->fileformat.c_str(), Qt::EditRole);
        index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 2, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(
            index, msToNiceStringExact(song_length_ms, m_displayMilliseconds), Qt::EditRole);
        index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 5, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index, song_length_ms, Qt::EditRole);
        index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 8, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index, artist, Qt::EditRole);

        index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 6, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index, false, Qt::EditRole);

        currentSubsong = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 3).data().toInt();


        QModelIndex index2 = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0);
        tableWidgetPlaylists[currentPlaylist]->scrollTo(index2);

        //        //Set text of currently playing sound to main color
        highlightPlaylistItem(currentPlaylist, currentRow);

        //        //Set text of currently playing playlist to main color
        QList<QListWidgetItem*> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        l.at(0)->setForeground(QColor(colorMain.left(7)));

        ui->positionSlider->setEnabled(SoundManager::getInstance().m_Info1->getSeekable());

        updateInstruments();


        //        //OK, now to add all subsongs, one for each row


        if (tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 3).data().toInt() < 1 &&
            SoundManager::getInstance().m_Info1->numSubsongs > 1)
        {
            QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 3, QModelIndex());
            tableWidgetPlaylists[currentPlaylist]->model()->setData(index, 1, Qt::EditRole);

            currentSubsong = 1;
            QString title = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0).data().toString();
            QString fileformat = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 1).data().toString();
            QString fullpath = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 4).data().toString();
            QString artist = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 8).data().toString();
            for (int i = 1; i < SoundManager::getInstance().m_Info1->numSubsongs; i++)
            {
                addPlaylistEntry(tableWidgetPlaylists[currentPlaylist], currentRow + i, title, fileformat, "", i + 1,
                                 fullpath, 0, artist);
            }
        }
        QModelIndex index3 = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 7, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index3, true, Qt::EditRole);
        tableWidgetPlaylists[currentPlaylist]->update();


        QString printerText = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0).data().toString();
        //        if(printerText.isEmpty())
        //        {
        //            printerText=SoundManager::getInstance().m_Info1->filename.c_str();
        //        }


        updateScrollText();


        ui->visualizer->getEffect()->setPrinterText(printerText);
    }
    else
    {
        addDebugText("Error loading file.");
    }
    updateButtons();
}


void MainWindow::updateInstruments()
{
    int row = 0;

    ui->instruments->clear();
    ui->instruments->setColumnCount(0);
    ui->instruments->setRowCount(0);
    ui->instruments->horizontalHeader()->setStretchLastSection(false);

    ui->samples->clear();
    ui->samples->setColumnCount(0);
    ui->samples->setRowCount(0);
    ui->samples->horizontalHeader()->setStretchLastSection(false);

    if (SoundManager::getInstance().m_Info1 != nullptr)
    {
        if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsSamples;
                columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend") <<
                    tr("Volume") << tr("Finetune") /*<< tr("Resolution") << tr("Channels")*/;

                ui->samples->setColumnCount(columnLabelsSamples.size());

                ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);
                ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
                ui->samples->setColumnWidth(3, sampleColumnLoopStartWidth);
                ui->samples->setColumnWidth(4, sampleColumnLoopEndWidth);
                ui->samples->setColumnWidth(5, sampleColumnVolumeWidth);
                ui->samples->setColumnWidth(6, sampleColumnFinetuneWidth);

                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(6)->setTextAlignment(Qt::AlignLeft);

                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                    unsigned int size = SoundManager::getInstance().m_Info1->samplesSize[j];
                    if (size > 0)
                    {
                        ui->samples->setItem(
                            j, 2, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesSize[j])));
                        ui->samples->setItem(j, 3, new QTableWidgetItem(
                                                 QString::number(
                                                     SoundManager::getInstance().m_Info1->samplesLoopStart[j])));
                        ui->samples->setItem(
                            j, 4, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesLoopEnd[j])));
                        ui->samples->setItem(
                            j, 5, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesVolume[j])));
                        ui->samples->setItem(
                            j, 6, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesFineTune[j])));
                    }
                }
            }
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name")
                    /*<< tr("Volume") << tr("Wavelen") << tr("Attack") << tr("Attack Volume") << tr("Decay") << tr("Decay Volume") << tr("Sustain") << tr("Sustain Volume") << tr("Release") << tr("Release Volume") << tr("Delay") << tr("Delay Depth") << tr("Delay Speed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed")*/
                    ;
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_uade)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsSamples;
                columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Volume");
                ui->samples->setColumnCount(columnLabelsSamples.size());
                ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);
                ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
                ui->samples->setColumnWidth(3, sampleColumnVolumeWidth);

                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);

                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                    unsigned int size = SoundManager::getInstance().m_Info1->samplesSize[j];
                    if (size > 0)
                    {
                        ui->samples->setItem(
                            j, 2, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesSize[j])));
                        ui->samples->setItem(
                            j, 3, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesVolume[j])));
                    }
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsSamples;
                columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend") <<
                    tr("Volume") << tr("Finetune") /*<< tr("Resolution") << tr("Channels")*/;
                ui->samples->setColumnCount(columnLabelsSamples.size());
                ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);
                ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
                ui->samples->setColumnWidth(3, sampleColumnLoopStartWidth);
                ui->samples->setColumnWidth(4, sampleColumnLoopEndWidth);
                ui->samples->setColumnWidth(5, sampleColumnVolumeWidth);
                ui->samples->setColumnWidth(6, sampleColumnFinetuneWidth);

                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(6)->setTextAlignment(Qt::AlignLeft);


                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                    unsigned int size = SoundManager::getInstance().m_Info1->samplesSize[j];
                    if (size > 0)
                    {
                        ui->samples->setItem(
                            j, 2, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesSize[j])));
                        ui->samples->setItem(j, 3, new QTableWidgetItem(
                                                 QString::number(
                                                     SoundManager::getInstance().m_Info1->samplesLoopStart[j])));
                        ui->samples->setItem(
                            j, 4, new QTableWidgetItem(
                                QString::number(SoundManager::getInstance().m_Info1->samplesLoopEnd[j])));
                        //these two probably should only be divided for mods (not xm etc.)
                        ui->samples->setItem(j, 5, new QTableWidgetItem(
                                                 QString::number(
                                                     SoundManager::getInstance().m_Info1->samplesVolume[j] / 4)));
                        ui->samples->setItem(j, 6, new QTableWidgetItem(
                                                 QString::number(
                                                     SoundManager::getInstance().m_Info1->samplesFineTune[j] / 16)));
                    }
                }
            }

            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name") << tr("Volume") << tr("Wavelen")
                    /*<< tr("Attack") << tr("Attack Volume") << tr("Decay") << tr("Decay Volume") << tr("Sustain") << tr("Sustain Volume") << tr("Release") << tr("Release Volume") << tr("Delay") << tr("Delay Depth") << tr("Delay Speed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed")*/
                    ;
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
                ui->instruments->setColumnWidth(2, instrumentColumnVolumeWidth);
                ui->instruments->setColumnWidth(3, instrumentColumnWaveLengthWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker)
        {
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name") << tr("Volume") << tr("Wavelen");
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
                ui->instruments->setColumnWidth(2, instrumentColumnVolumeWidth);
                ui->instruments->setColumnWidth(3, instrumentColumnWaveLengthWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);


                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                    ui->instruments->setItem(
                        j, 2, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->instrumentsVolume[j])));
                    ui->instruments->setItem(
                        j, 3, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->instrumentsWavelen[j])));
                }
            }
        }

        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_protrekkr)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsSamples;
                columnLabelsSamples << tr("#") << tr("Name");
                ui->samples->setColumnCount(columnLabelsSamples.size());
                ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);

                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                }
            }
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name");
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }

        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_klystron)
        {
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name");
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_asap)
        {
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name");
                ui->instruments->setColumnCount(columnLabelsInstruments.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libpac)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend") <<
                    tr("Volume") << tr("Finetune") << tr("Resolution");
                ui->samples->setColumnCount(columnLabelsInstruments.size());
                ui->samples->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);
                ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
                ui->samples->setColumnWidth(3, sampleColumnLoopStartWidth);
                ui->samples->setColumnWidth(4, sampleColumnLoopEndWidth);
                ui->samples->setColumnWidth(5, sampleColumnVolumeWidth);
                ui->samples->setColumnWidth(6, sampleColumnFinetuneWidth);
                ui->samples->setColumnWidth(7, sampleColumnResolutionWidth);

                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(6)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(7)->setTextAlignment(Qt::AlignLeft);


                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                    QString res = "8-bit";
                    if (SoundManager::getInstance().m_Info1->samples16Bit[j])
                    {
                        res = "16-bit";
                    }
                    ui->samples->setItem(j, 7, new QTableWidgetItem(res));

                    ui->samples->setItem(
                        j, 2,
                        new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->samplesSize[j])));
                    ui->samples->setItem(
                        j, 3, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->samplesLoopStart[j])));
                    ui->samples->setItem(
                        j, 4, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->samplesLoopEnd[j])));
                    ui->samples->setItem(
                        j, 5, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->samplesVolume[j])));
                    ui->samples->setItem(
                        j, 6, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->samplesFineTune[j])));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_flod)
        {
            if (SoundManager::getInstance().m_Info1->numSamples > 0)
            {
                QStringList columnLabelsInstruments;
                columnLabelsInstruments << tr("#") << tr("Name") << tr("Size") << tr("Volume");
                ui->samples->setColumnCount(columnLabelsInstruments.size());
                ui->samples->setHorizontalHeaderLabels(columnLabelsInstruments);
                ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
                ui->samples->setColumnWidth(1, sampleColumnNameWidth);
                ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
                ui->samples->setColumnWidth(3, sampleColumnVolumeWidth);


                ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
                ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);

                ui->samples->setRowCount(SoundManager::getInstance().m_Info1->numSamples);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numSamples; j++)
                {
                    ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->samples->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->samples[j].c_str()));
                    ui->samples->setItem(
                        j, 2,
                        new QTableWidgetItem(QString::number(SoundManager::getInstance().m_Info1->samplesSize[j])));
                    ui->samples->setItem(
                        j, 3, new QTableWidgetItem(
                            QString::number(SoundManager::getInstance().m_Info1->samplesVolume[j])));
                }
            }
        }
        else if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_adplug)
        {
            if (SoundManager::getInstance().m_Info1->numInstruments > 0)
            {
                QStringList columnLabelsSamples;
                columnLabelsSamples << tr("#") << tr("Name");
                ui->instruments->setColumnCount(columnLabelsSamples.size());
                ui->instruments->setHorizontalHeaderLabels(columnLabelsSamples);
                ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
                ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);

                ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
                ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

                ui->instruments->setRowCount(SoundManager::getInstance().m_Info1->numInstruments);
                for (int j = 0; j < SoundManager::getInstance().m_Info1->numInstruments; j++)
                {
                    ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
                    ui->instruments->setItem(
                        j, 1, new QTableWidgetItem(SoundManager::getInstance().m_Info1->instruments[j].c_str()));
                }
            }
        }
    }
    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
}


void MainWindow::restoreLayout()
{
    restoreGeometry(defaultGeometry);
    restoreState(defaultState);
    ui->dockWidget->hide();
    ui->dockWidgetChannels->hide();
    ui->dockWidgetFilename->hide();
    ui->dockWidgetInfo->hide();
    ui->dockWidgetInstruments->hide();
    ui->dockWidgetLogger->hide();
    ui->dockWidgetPlaylist->hide();
    ui->dockWidgetPlaylists->hide();
    ui->dockWidgetSamples->hide();
    ui->dockWidgetTrackerView->hide();
    ui->dockWidgetVisualizer->hide();
    m_DockManager->restoreState(dockingState);
}

void MainWindow::exportInstrumentToWAV()
{
    exportInstrument("WAV");
}

void MainWindow::muteAllChannels()
{
    channels->muteAllChannels();
}

void MainWindow::unmuteAllChannels() const
{
    channels->unmuteAllChannels();
}

void MainWindow::exportInstrumentToIFF()
{
    exportInstrument("IFF");
}

void MainWindow::exportInstrument(QString format)
{
    int row = ui->samples->currentRow();

    if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp && ui->samples->rowCount() > 0 &&
        SoundManager::getInstance().m_Info1->samplesSize[row] > 0)
    {
        QString defaultFileName = SoundManager::getInstance().m_Info1->samples[row].c_str();
        defaultFileName.replace(QRegularExpression(":|;|/|\""), "-");

        unsigned int loopStart = SoundManager::getInstance().m_Info1->samplesLoopStart[row];
        unsigned int loopLength = SoundManager::getInstance().m_Info1->samplesLoopEnd[row] - SoundManager::getInstance()
            .m_Info1->samplesLoopStart[row];

        if (format == "WAV")
        {
            QString fileName = QFileDialog::getSaveFileName(this, "Export sample", "/" + defaultFileName,
                                                            "Wave (*.wav)");


            addDebugText(
                "Saving sample to WAV, no. " + QString::number(row) + ", filesize: " + QString::number(
                    SoundManager::getInstance().m_Info1->samplesSize[row]));

            typedef struct wavHeader_t
            {
                uint32_t chunkID, chunkSize, format, subchunk1ID, subchunk1Size;
                uint16_t audioFormat, numChannels;
                uint32_t sampleRate, byteRate;
                uint16_t blockAlign, bitsPerSample;
                uint32_t subchunk2ID, subchunk2Size;
            } wavHeader_t;

            typedef struct sampleLoop_t
            {
                uint32_t dwIdentifier, dwType, dwStart;
                uint32_t dwEnd, dwFraction, dwPlayCount;
            } sampleLoop_t;

            typedef struct samplerChunk_t
            {
                uint32_t chunkID, chunkSize, dwManufacturer, dwProduct;
                uint32_t dwSamplePeriod, dwMIDIUnityNote, wMIDIPitchFraction;
                uint32_t dwSMPTEFormat, dwSMPTEOffset, cSampleLoops, cbSamplerData;
                sampleLoop_t loop;
            } samplerChunk_t;

            wavHeader_t wavHeader;
            samplerChunk_t samplerChunk;

            wavHeader.format = 0x45564157; // "WAVE"
            wavHeader.chunkID = 0x46464952; // "RIFF"
            wavHeader.subchunk1ID = 0x20746D66; // "fmt "
            wavHeader.subchunk2ID = 0x61746164; // "data"
            wavHeader.subchunk1Size = 16;
            wavHeader.subchunk2Size = SoundManager::getInstance().m_Info1->samplesSize[row];
            wavHeader.chunkSize = 36 + wavHeader.subchunk2Size;
            wavHeader.audioFormat = 1;
            wavHeader.numChannels = 1;
            wavHeader.bitsPerSample = 8;
            wavHeader.sampleRate = 16574;
            wavHeader.byteRate = wavHeader.sampleRate * wavHeader.numChannels * wavHeader.bitsPerSample / 8;
            wavHeader.blockAlign = wavHeader.numChannels * wavHeader.bitsPerSample / 8;

            // set "sampler" chunk if loop is enabled
            if (loopStart + loopLength > 2) // loop enabled?
            {
                addDebugText("Loop is enabled.");
                wavHeader.chunkSize += sizeof(samplerChunk_t);
                memset(&samplerChunk, 0, sizeof(samplerChunk_t));
                samplerChunk.chunkID = 0x6C706D73; // "smpl"
                samplerChunk.chunkSize = 60;
                samplerChunk.dwSamplePeriod = 1000000000 / 16574;
                samplerChunk.dwMIDIUnityNote = 60; // 60 = MIDI middle-C
                samplerChunk.cSampleLoops = 1;
                samplerChunk.loop.dwStart = loopStart;
                samplerChunk.loop.dwEnd = (loopStart + loopLength) - 1;
            }


            FILE* pFile;
            pFile = fopen(fileName.toStdString().c_str(), "wb");
            fwrite((char*)&wavHeader, sizeof(char), sizeof wavHeader, pFile);
            for (int i = 0; i < SoundManager::getInstance().m_Info1->samplesSize[row]; i++)
                fputc((uint8_t)(SoundManager::getInstance().m_Info1->samplesData[row][i] + 128), pFile);

            if (SoundManager::getInstance().m_Info1->samplesSize[row] & 1)
                fputc(0, pFile); // pad align byte

            if (loopStart + loopLength > 2) // loop enabled?
                fwrite(&samplerChunk, sizeof (samplerChunk), 1, pFile);

            fclose(pFile);
        }
        else if (format == "IFF")
        {
            addDebugText(
                "Saving sample to IFF, no. " + QString::number(row) + ", filesize: " + QString::number(
                    SoundManager::getInstance().m_Info1->samplesSize[row]));

            QString fileName =
                QFileDialog::getSaveFileName(this, "Export sample", "/" + defaultFileName, "IFF (*.iff)");

            FILE* f;
            f = fopen(fileName.toStdString().c_str(), "wb");

            // "FORM" chunk
            iffWriteChunkHeader(f, "FORM", 0); // "FORM" chunk size is overwritten later
            iffWriteUint32(f, 0x38535658); // "8SVX"

            // "VHDR" chunk
            iffWriteChunkHeader(f, "VHDR", 20);

            if (loopStart + loopLength > 2) // loop enabled?
            {
                iffWriteUint32(f, loopStart); // oneShotHiSamples
                iffWriteUint32(f, loopLength); // repeatHiSamples
            }
            else
            {
                iffWriteUint32(f, 0); // oneShotHiSamples
                iffWriteUint32(f, 0); // repeatHiSamples
            }

            iffWriteUint32(f, 0); // samplesPerHiCycle
            iffWriteUint16(f, 16574); // samplesPerSec
            iffWriteUint8(f, 1); // ctOctave (number of samples)
            iffWriteUint8(f, 0); // sCompression
            //iffWriteUint32(f, s->volume * 1024); // volume (max: 65536/0x10000)
            iffWriteUint32(f, 1 * 1024); // volume (max: 65536/0x10000)

            uint32_t chunkLen;
            // "NAME" chunk
            chunkLen = (uint32_t)strlen(SoundManager::getInstance().m_Info1->samples[row].c_str());
            if (chunkLen > 0)
            {
                iffWriteChunkHeader(f, "NAME", chunkLen);
                iffWriteChunkData(f, SoundManager::getInstance().m_Info1->samples[row].c_str(), chunkLen);
            }

            // "ANNO" chunk (we put the program name here)
            const char annoStr[] = PROJECT_NAME;
            chunkLen = sizeof (annoStr) - 1;
            iffWriteChunkHeader(f, "ANNO", chunkLen);
            iffWriteChunkData(f, annoStr, chunkLen);

            // "BODY" chunk
            chunkLen = SoundManager::getInstance().m_Info1->samplesSize[row];
            iffWriteChunkHeader(f, "BODY", chunkLen);
            iffWriteChunkData(f, SoundManager::getInstance().m_Info1->samplesData[row], chunkLen);

            // go back and fill in "FORM" chunk size
            chunkLen = ftell(f) - 8;
            fseek(f, 4, SEEK_SET);
            iffWriteUint32(f, chunkLen);

            fclose(f);
        }
    }
}

void MainWindow::fullscreenVisualizer()
{
    visualizerFullScreen->showFullScreen();
}

void MainWindow::showNextVisualizer()
{
    visualizerFullScreen->showFullScreen();
}

void MainWindow::fullscreenTracker()
{
    trackerFullScreen->showFullScreen();
}

void MainWindow::selectAllLogWindow() const
{
    ui->Debug->selectAll();
}

void MainWindow::copyLogWindow() const
{
    ui->Debug->copy();
}

void MainWindow::clearLogWindow() const
{
    ui->Debug->clear();
}

void MainWindow::renamePlaylist()
{
    if (ui->listWidget->currentItem()->text() == PLAYLIST_DEFAULT_FILENAME) return;

    bool ok;
    QString oldName = ui->listWidget->currentItem()->text();
    QString newName = QInputDialog::getText(this, "Rename Playlist",
                                            "New name:", QLineEdit::Normal,
                                            oldName, &ok);

    if ((oldName != newName) && ok && !newName.isEmpty()) {
        if (!newName.endsWith(PLAYLIST_DEFAULT_EXTENSION)) {
            newName = newName + PLAYLIST_DEFAULT_EXTENSION;
        }
        QString newOrgFilename = newName;
        int suffix = 0;
        while (tableWidgetPlaylists.contains(newName)) {
            suffix++;
            newName = newOrgFilename + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
        }
        QDir directory(userPath + PLAYLISTS_DIR);
        QString playlistNewName = userPath + PLAYLISTS_DIR + QDir::separator() + newName;
        QString playlistOldName = userPath + PLAYLISTS_DIR + QDir::separator() + oldName;
        QFile oldFile(playlistOldName);
        oldFile.rename(playlistNewName);
        addDebugText("Renaming playlist " + playlistOldName + " to " + playlistNewName);
        ui->listWidget->currentItem()->setText(newName);

        if (oldName == currentPlaylist) {
            currentPlaylist = newName;
        }
        QTableView *newTableWidget = tableWidgetPlaylists[oldName];
        tableWidgetPlaylists.remove(oldName);
        tableWidgetPlaylists.insert(newName, newTableWidget);
        DockWidgetPlaylist->setWindowTitle(newName);
    }
}

void MainWindow::savePlaylistAs()
{
    bool ok;
    QString oldName = ui->listWidget->currentItem()->text();
    QString newName = QInputDialog::getText(this, "Save Playlist As",
                                            "New name:", QLineEdit::Normal,
                                            oldName, &ok);

    if (oldName == newName || !ok || newName.isEmpty()) return;

    if (!newName.endsWith(PLAYLIST_DEFAULT_EXTENSION)) {
        newName = newName + PLAYLIST_DEFAULT_EXTENSION;
    }
    QString newOrgFilename = newName;
    int suffix = 0;
    while (tableWidgetPlaylists.contains(newName)) {
        suffix++;
        newName = newOrgFilename + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
    }
    QDir directory(userPath + PLAYLISTS_DIR);
    QString playlistNewName = userPath + PLAYLISTS_DIR + QDir::separator() + newName;
    QString playlistOldName = userPath + PLAYLISTS_DIR + QDir::separator() + oldName;

    savePlayList(playlistOldName, playlistNewName);

    addDebugText("Saving playlist: " + playlistOldName + " as " + playlistNewName);
    QListWidgetItem *newItem = ui->listWidget->currentItem()->clone();
    newItem->setText(newName);
    newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));

    ui->listWidget->addItem(newItem);
    MyItemDelegate* item = new MyItemDelegate(this);
    item->setMainColor(QColor(colorMain.left(7)));
    ui->listWidget->setItemDelegate(item);

    DraggableTableView *tv = new DraggableTableView();
    tv->setDragBackgroundColor(QColor(colorMain.left(7)));
    tv->setDragTextColor(QColor(colorMainText.left(7)));
    tv->setupDelegate(); // has to be called after colors are set
    PlaylistModel* pm = new PlaylistModel(this);
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(pm); // create proxy
    proxyModel->setSourceModel(pm);
    tv->setModel(proxyModel);

    tv->setColumnHidden(4, true);
    tv->setColumnHidden(5, true);
    tv->setColumnHidden(6, true);
    tv->setColumnHidden(7, true);

    tableWidgetPlaylists[newName] = tv;

    QFont roboto("Roboto");


    tableWidgetPlaylists[newName]->setStyleSheet(
        ui->dockWidgetContents_4->styleSheet() +
        "QHeaderView::section{font-family:Roboto;padding:0;} QTableView{padding:9px;}");

    tableWidgetPlaylists[newName]->setFont(roboto);
    tableWidgetPlaylists[newName]->installEventFilter(this);

    swapColumns(tableWidgetPlaylists[newName]);


    connect(tableWidgetPlaylists[newName], SIGNAL(doubleClicked(const QModelIndex &)),
            SLOT(on_playlist_itemDoubleClicked(const QModelIndex &)));

    QUrl u = QUrl::fromLocalFile(
        QDir::separator() + userPath + PLAYLISTS_DIR + "/" + newName);
    QList<QUrl> ql;
    ql.append(u);

    addSong(ql, 0, newName, false);

    createThePopupMenuCurrentPlaylist(newName);
}

void MainWindow::savePlayList(QString path, QString newPath)
{
    QFileInfo fileInfoOld(path);
    QFileInfo fileInfoNew(newPath);
    QFile file(newPath);
    QDir dir(newPath);
    if (!file.open(QIODevice::WriteOnly))
        return;
    QTextStream out(&file);

    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);

    out << "#EXTM3U\n";
    addDebugText("Saving playlist: " + newPath);

    for (int i = 0; i < tableWidgetPlaylists[fileInfoOld.fileName()]->model()->rowCount(); i++)
    {
        QString playlistKey = fileInfoOld.fileName();
        QString path = tableWidgetPlaylists[playlistKey]->model()->index(i, 4).data().toString();
        QString extInfo = "#EXTINF:" +
            QString::number(tableWidgetPlaylists[playlistKey]->model()->index(i, 5).data().toInt()) + "," + QString(
                tableWidgetPlaylists[playlistKey]->model()->index(i, 0).data().toString());
        out << extInfo << "\n" << path << NEZPLAYLISTSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 1)
            .data().toString() << PLAYLISTFIELDSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 3).
            data().toString() << PLAYLISTFIELDSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 0).data()
            .toString() << PLAYLISTFIELDSPLITTER << "[time(h:m:s)] " << PLAYLISTFIELDSPLITTER << "[loop(h:m:s)][-]" <<
            PLAYLISTFIELDSPLITTER << "[fade(h:m:s)]" << PLAYLISTFIELDSPLITTER << "[loopcount]" << PLAYLISTFIELDSPLITTER
            << tableWidgetPlaylists[playlistKey]->model()->index(i, 8).data().toString() << "\n";
    }
    file.close();
}

void MainWindow::savePlaylist() {
    savePlayList(userPath + PLAYLISTS_DIR + "/" + ui->listWidget->currentItem()->text(),
                 userPath + PLAYLISTS_DIR + "/" + ui->listWidget->currentItem()->
                 text());
}

void MainWindow::deleteAllPlaylists()
{
    for (int rowNumber = ui->listWidget->count() - 1; rowNumber >= 0; rowNumber--)
    {
        ui->listWidget->setCurrentRow(rowNumber);
        if (ui->listWidget->currentItem()->text() != PLAYLIST_DEFAULT_FILENAME)
        {
            QString playlistToDelete = userPath + PLAYLISTS_DIR + QDir::separator() + ui->listWidget->currentItem()
                                       ->text();
            addDebugText("Deleting playlist " + playlistToDelete);
            QFile::remove(playlistToDelete);
            tableWidgetPlaylists.remove(ui->listWidget->currentItem()->text());
            delete ui->listWidget->takeItem(ui->listWidget->currentRow());
        }
    }
    ui->listWidget->setCurrentRow(0);
    on_listWidget_itemClicked(ui->listWidget->item(0));
}

void MainWindow::deletePlaylist()
{
    if (ui->listWidget->currentItem()->text() == PLAYLIST_DEFAULT_FILENAME) return;

    int rowNumber = ui->listWidget->currentRow();
    rowNumber--;

    QString playlistToDelete = userPath + PLAYLISTS_DIR + QDir::separator() + ui->listWidget->currentItem()->text();
    addDebugText("Deleting playlist " + playlistToDelete);
    QFile::remove(playlistToDelete);
    //playlists.remove(ui->listWidget->currentItem()->text());
    tableWidgetPlaylists.remove(ui->listWidget->currentItem()->text());
    delete ui->listWidget->takeItem(ui->listWidget->currentRow());
    ui->listWidget->setCurrentRow(rowNumber);
    on_listWidget_itemClicked(ui->listWidget->item(rowNumber));
}

void MainWindow::clearPlaylist()
{
    addDebugText("Clears playlist " + selectedPlaylist);
    removeHighlight();
    currentRow = 0;
    tableWidgetPlaylists[selectedPlaylist]->model()->removeRows(
        0, tableWidgetPlaylists[selectedPlaylist]->model()->rowCount());

    on_buttonStop_clicked();
    isUpdateCurrentRowToNextEnabled = false;

    if (selectedPlaylist == currentPlaylist)
    {
        resetShuffle(currentPlaylist);
    }
}

void MainWindow::showContainingFolder()
{
    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows())
    {
        QFile file(tableWidgetPlaylists[selectedPlaylist]->model()->index(idx.row(), 4).data().toString());
        QFileInfo fileinfo(file);
        if (!fileinfo.path().startsWith("http://", Qt::CaseInsensitive) && !fileinfo.path().startsWith("https://"))
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.path()));
        }
    }
}

void MainWindow::deleteFilesInPlaylist()
{
    bool rowDeleted = false;
    QVector<int> selectedRowsIdx;

    addDebugText("Removing items from playlist: " + selectedPlaylist);
    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows())
    {
        selectedRowsIdx.append(idx.row());
        addDebugText("Removing items from playlist: " + QString::number(idx.row()));
    }

    std::sort(selectedRowsIdx.begin(), selectedRowsIdx.end());

    int currentRowPreDelete = currentRow;

    for (int idx = selectedRowsIdx.size() - 1; idx >= 0; idx--)
    {
        tableWidgetPlaylists[selectedPlaylist]->model()->removeRow(selectedRowsIdx.at(idx));
        rowDeleted = true;

        if (selectedRowsIdx.at(idx) < currentRow && currentRow != 0)
        {
            currentRow--;
        }
        addDebugText("Removing row: " + QString::number(selectedRowsIdx.at(idx)));
    }

    if (rowDeleted)
    {
        addDebugText("Row(s) in playlist deleted.");

        if (tableWidgetPlaylists[selectedPlaylist]->model()->rowCount() == 0)
        {
            on_buttonStop_clicked();
            isUpdateCurrentRowToNextEnabled = false;
        }
        else
        {
            for (int idx = selectedRowsIdx.size() - 1; idx >= 0; idx--)
            {
                if (selectedRowsIdx.at(idx) == currentRowPreDelete)
                {
                    on_buttonStop_clicked();
                    isUpdateCurrentRowToNextEnabled = false;
                    break;
                }
            }
        }

        if (selectedPlaylist == currentPlaylist)
        {
            resetShuffle(currentPlaylist);
        }
    }
}

void MainWindow::deleteFilesInvertedInPlaylist()
{
    bool rowDeleted = false;

    addDebugText("Removing items from playlist: " + selectedPlaylist);

    //get all selected rows
    QVector<int> selectedRowsIdx;
    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows())
    {
        selectedRowsIdx.append(idx.row());
    }

    //get all NOT selected rows
    QVector<int> notSelectedRowsIdx;
    for (int i = 0; i < tableWidgetPlaylists[selectedPlaylist]->model()->rowCount(); i++)
    {
        if (!selectedRowsIdx.contains(i))
        {
            notSelectedRowsIdx.append(i);
        }
    }


    for (int idx = notSelectedRowsIdx.size() - 1; idx >= 0; idx--)
    {
        tableWidgetPlaylists[selectedPlaylist]->model()->removeRow(notSelectedRowsIdx.at(idx));
        rowDeleted = true;
        if (currentRow >= notSelectedRowsIdx.at(idx) && currentRow != 0)
        {
            currentRow--;
        }
    }

    if (rowDeleted)
    {
        addDebugText("Row(s) in playlist deleted.");

        if (selectedPlaylist == currentPlaylist)
        {
            resetShuffle(currentPlaylist);
        }
    }
}


void MainWindow::openSettings()
{
    settingsWindow settingsWindow(this);
    settingsWindow.exec();
}

void MainWindow::addFolder()
{
    QUrl root = QUrl(lastDir);
    QUrl u = QFileDialog::getExistingDirectoryUrl(this, "Add folder", root);
    if (!u.isEmpty())
    {
        QList<QUrl> ql;
        ql.append(u);
        addSong(ql, 0, ui->listWidget->currentItem()->text(), true);
        lastDir = u.toLocalFile();
    }
}

void MainWindow::addFiles()
{
    QString root = lastDir;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add files", root, tr("All files (*.*)"));
    if (fileNames.size() > 0)
    {
        addSong(fileNames, 0, selectedPlaylist, true);
        QFileInfo a(fileNames.last());
        lastDir = a.absolutePath();
    }
}

void MainWindow::setChannelEnabled(int index, bool enable)
{
    channels->setChannelEnabled(index, enable);
}

bool MainWindow::getChannelEnabled(int index)
{
    return channels->getChannelEnabled(index);
}

void MainWindow::createThePopupLogWindow()
{
    selectAllLogWindowAction = new QAction(tr("&Select All"), this);
    copyLogWindowAction = new QAction(tr("&Copy"), this);
    clearLogWindowAction = new QAction(tr("C&lear"), this);

    connect(clearLogWindowAction, SIGNAL(triggered()), this, SLOT(clearLogWindow()));
    connect(copyLogWindowAction, SIGNAL(triggered()), this, SLOT(copyLogWindow()));
    connect(selectAllLogWindowAction, SIGNAL(triggered()), this, SLOT(selectAllLogWindow()));

    ui->Debug->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->Debug->addActions({clearLogWindowAction});
    ui->Debug->addActions({copyLogWindowAction});
    ui->Debug->addActions({selectAllLogWindowAction});
}

/*!
 Creates the popup menu for playlists window
*/
void MainWindow::createThePopupMenuPlaylists()
{
    renamePlaylistAction = new QAction(tr("&Rename"), this);
    clearPlaylistAction = new QAction(tr("&Clear"), this);
    deletePlaylistAction = new QAction(tr("&Delete"), this);
    deleteAllPlaylistsAction = new QAction(tr("D&elete All Playlists"), this);
    savePlaylistAction = new QAction(tr("&Save"), this);
    savePlaylistAsAction = new QAction(tr("S&ave As"), this);

    connect(renamePlaylistAction, SIGNAL(triggered()), this, SLOT(renamePlaylist()));
    connect(clearPlaylistAction, SIGNAL(triggered()), this, SLOT(clearPlaylist()));
    connect(deletePlaylistAction, SIGNAL(triggered()), this, SLOT(deletePlaylist()));
    connect(deleteAllPlaylistsAction, SIGNAL(triggered()), this, SLOT(deleteAllPlaylists()));
    connect(savePlaylistAction, SIGNAL(triggered()), this, SLOT(savePlaylist()));
    connect(savePlaylistAsAction, SIGNAL(triggered()), this, SLOT(savePlaylistAs()));

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QListWidget* list = ui->listWidget;
    QObject::connect(list, &QListWidget::customContextMenuRequested, [list, this](const QPoint& pos)
    {
        QModelIndex index = list->indexAt(pos);

        if (list->itemAt(pos) != nullptr)
        {
            if (index.row() == 0)
            {
                QMenu rightClickMenu(list);
                rightClickMenu.addActions({
                    clearPlaylistAction, deleteAllPlaylistsAction, savePlaylistAction, savePlaylistAsAction
                });
                rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
            }
            else
            {
                QMenu rightClickMenu(list);
                rightClickMenu.addActions({
                    renamePlaylistAction, clearPlaylistAction, deletePlaylistAction, deleteAllPlaylistsAction,
                    savePlaylistAction, savePlaylistAsAction
                });
                rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
            }
        }
        else
        {
            QMenu rightClickMenu(list);
            rightClickMenu.addActions({deleteAllPlaylistsAction});
            rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
        }
    });
}

void MainWindow::createThePopupMenuVisualizer()
{
    fullscreenVisualizerAction = new QAction(tr("&Show Fullscreen"), this);

    showNextVisualizerAction = new QAction(tr("S&how Next Visualizer"), this);

    connect(fullscreenVisualizerAction, SIGNAL(triggered()), this, SLOT(fullscreenVisualizer()));
    connect(showNextVisualizerAction, SIGNAL(triggered()), this, SLOT(showNextVisualizer()));


    ui->visualizer->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->visualizer->addActions({fullscreenVisualizerAction});
}

void MainWindow::createThePopupMenuTracker()
{
    fullscreenTrackerAction = new QAction(tr("&Show Fullscreen"), this);

    connect(fullscreenTrackerAction, SIGNAL(triggered()), this, SLOT(fullscreenTracker()));

    ui->trackerView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->trackerView->addActions({fullscreenTrackerAction});
}

/*!
 Creates the popup menu for current playlist window
*/
void MainWindow::createThePopupMenuCurrentPlaylist(QString playlist)
{
    deleteFilesInPlaylistAction = new QAction(tr("&Remove selected"), this);
    deleteFilesInPlaylistInvertedAction = new QAction(tr("&Remove all not selected"), this);
    showContainingFolderAction = new QAction(tr("&Show containing folder"), this);
    clearPlaylistAction = new QAction(tr("&Clear playlist"), this);

    connect(deleteFilesInPlaylistAction, SIGNAL(triggered()), this, SLOT(deleteFilesInPlaylist()));
    connect(deleteFilesInPlaylistInvertedAction, SIGNAL(triggered()), this, SLOT(deleteFilesInvertedInPlaylist()));
    connect(showContainingFolderAction, SIGNAL(triggered()), this, SLOT(showContainingFolder()));
    connect(clearPlaylistAction, SIGNAL(triggered()), this, SLOT(clearPlaylist()));

    tableWidgetPlaylists[playlist]->setContextMenuPolicy(Qt::ActionsContextMenu);
    tableWidgetPlaylists[playlist]->addActions({deleteFilesInPlaylistAction});
    tableWidgetPlaylists[playlist]->addActions({deleteFilesInPlaylistInvertedAction});
    tableWidgetPlaylists[playlist]->addActions({showContainingFolderAction});
    tableWidgetPlaylists[playlist]->addActions({clearPlaylistAction});
}

void MainWindow::createThePopupMenuInstruments()
{
    exportInstrumentWAVAction = new QAction(tr("&Export to .wav"), this);
    exportInstrumentIFFAction = new QAction(tr("&Export to .iff"), this);

    connect(exportInstrumentWAVAction, SIGNAL(triggered()), this, SLOT(exportInstrumentToWAV()));
    connect(exportInstrumentIFFAction, SIGNAL(triggered()), this, SLOT(exportInstrumentToIFF()));

    ui->samples->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->samples->addActions({exportInstrumentWAVAction});
    ui->samples->addActions({exportInstrumentIFFAction});
}

void MainWindow::createThePopupMenuChannels()
{
    muteAllChannelsAction = new QAction(tr("&Mute All Channels"), this);
    unmuteAllChannelsAction = new QAction(tr("&Unmute All Channels"), this);

    connect(muteAllChannelsAction, SIGNAL(triggered()), this, SLOT(muteAllChannels()));
    connect(unmuteAllChannelsAction, SIGNAL(triggered()), this, SLOT(unmuteAllChannels()));

    ui->dockWidgetContents_7->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->dockWidgetContents_7->addActions({muteAllChannelsAction});
    ui->dockWidgetContents_7->addActions({unmuteAllChannelsAction});
}

int MainWindow::getCurrentRow() const
{
    return currentRow;
}
void MainWindow::setCurrentRow(int row)
{
    currentRow=row;
}
void MainWindow::playPrevSong()
{
    addDebugText("Play previous song.");
    if ((currentRow != 0 && !isShuffleEnabled()) || (isShuffleEnabled() && m_iCurrentShufflePosition[currentPlaylist] > 0) ||
    (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked && !isShuffleEnabled()))
    {
        if (isShuffleEnabled())
        {
            m_iCurrentShufflePosition[currentPlaylist]--;
            removeHighlight();
            currentRow = m_ShufflePlayed[currentPlaylist].at(m_iCurrentShufflePosition[currentPlaylist]);
        }
        else if (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked && currentRow == 0)
        {
            removeHighlight();
            currentRow = tableWidgetPlaylists[currentPlaylist]->model()->rowCount() - 1;
        }
        else
        {
            removeHighlight();
            currentRow--;
        }


        PlaySong(currentRow);
        isUpdateCurrentRowToNextEnabled = true;
    }
}

void MainWindow::on_buttonNext_clicked()
{
    buttonNextClicked = true;
    playNextSong(true);
}

void MainWindow::on_buttonPrev_clicked()
{
    buttonNextClicked = false;
    playPrevSong();
}

void MainWindow::setOutputDeviceSetting(int outputDevice)
{
    m_outputDevice = outputDevice;
}

void MainWindow::setOutputDevice(int outputDevice, QString fullPath)
{
    {
        SoundManager::getInstance().ShutDown();
        SoundManager::getInstance().Init(outputDevice, fullPath);
        resetAll();
        setReverbEnabled(getReverbEnabled());
        setNormalizeEnabled(getNormalizeEnabled());
    }
    m_outputDevice = outputDevice;
}

int MainWindow::getOutputDevice() const
{
    return m_outputDevice;
}

void MainWindow::setResetVolume(bool resetVolume)
{
    m_resetVolume = resetVolume;
}

bool MainWindow::getResetVolume() const
{
    return m_resetVolume;
}

void MainWindow::setDefaultPlaymode(int defaultPlaymode)
{
    m_defaultPlaymode = defaultPlaymode;
}

int MainWindow::getDefaultPlaymode() const
{
    return m_defaultPlaymode;
}

void MainWindow::setReverbPreset(const QString& name)
{
    SoundManager::getInstance().setReverbPreset(name);
    m_reverbPreset = name;
}

void MainWindow::setIgnoreSuffix(QString suffix)
{
    m_ignoreSuffix = suffix;
}

void MainWindow::setIgnorePrefix(QString prefix)
{
    m_ignorePrefix = prefix;
}

QString MainWindow::getReverbPreset()
{
    return m_reverbPreset;
}

QString MainWindow::getIgnorePrefix()
{
    return m_ignorePrefix;
}

QString MainWindow::getIgnoreSuffix()
{
    return m_ignoreSuffix;
}

void MainWindow::setReverbEnabled(bool reverb)
{
    m_reverbEnabled = reverb;
    SoundManager::getInstance().setReverbEnabled(reverb);
}

bool MainWindow::getReverbEnabled() const
{
    return m_reverbEnabled;
}

void MainWindow::setNormalizeEnabled(bool normalize)
{
    m_normalizeEnabled = normalize;
    SoundManager::getInstance().setNormalizeEnabled(normalize);
}


bool MainWindow::getNormalizeEnabled() const
{
    return m_normalizeEnabled;
}

bool MainWindow::getDisplayMilliseconds() const
{
    return m_displayMilliseconds;
}

bool MainWindow::getEnqueueItems() const
{
    return m_enqueueItems;
}

bool MainWindow::getSystrayOnQuitEnabled() const
{
    return m_systrayOnQuitEnabled;
}

void MainWindow::setNormalizeFadeTime(int fadeTime)
{
    m_normalizeFadeTime = fadeTime;
    SoundManager::getInstance().setNormalizeFadeTime(fadeTime);
}

int MainWindow::getNormalizeFadeTime() const
{
    return m_normalizeFadeTime;
}

void MainWindow::setNormalizeThreshold(int threshold)
{
    SoundManager::getInstance().setNormalizeThreshold(threshold);
    m_normalizeThreshold = threshold;
}

int MainWindow::getNormalizeThreshold() const
{
    return m_normalizeThreshold;
}

void MainWindow::setNormalizeMaxAmp(int maxAmp)
{
    SoundManager::getInstance().setNormalizeMaxAmp(maxAmp);
    m_normalizeMaxAmp = maxAmp;
}

int MainWindow::getNormalizeMaxAmp() const
{
    return m_normalizeMaxAmp;
}

int MainWindow::getResetVolumeValue() const
{
    return m_resetVolumeValue;
}

int MainWindow::getPlaylistRowHeight() const
{
    return playlistRowHeight;
}

qint64 MainWindow::getHvscSonglengthsDownloaded() const
{
    return HvscSonglengthsDownloadedEpoch;
}

QString MainWindow::getHvscSonglengthsFrequency()
{
    return HvscSonglengthsFrequency;
}

QString MainWindow::getHvscSonglengthsPathDownloaded()
{
    return HvscSonglengthsPathDownloaded;
}

void MainWindow::setHvscSonglengthsPathDownloaded(QString path)
{
    HvscSonglengthsPathDownloaded = path;
}

int MainWindow::getPlaylistsRowHeight() const
{
    return playlistsRowHeight;
}

int MainWindow::getNowPlayingFontSize() const
{
    return nowPlayingFontSize;
}

void MainWindow::setResetVolumeValue(int value)
{
    m_resetVolumeValue = value;
}

void MainWindow::setSystrayOnQuitEnabled(bool enabled)
{
    m_systrayOnQuitEnabled = enabled;
}

void MainWindow::SaveSettings()
{
    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("outputdevice", m_outputDevice);
    settings.setValue("volume", ui->volumeSlider->value());
    settings.setValue("shuffle", ui->checkBoxShuffle->isChecked());

    settings.setValue("default_playmode", m_defaultPlaymode);
    settings.setValue("playmode", m_defaultPlaymode == -1 ? Playmode : m_defaultPlaymode);

    settings.setValue("resetvolume", m_resetVolume);
    settings.setValue("resetvolumevalue", m_resetVolumeValue);
    settings.setValue("mutevolume", ui->checkBoxVolumeOn->checkState() != Qt::Checked);


    settings.setValue("displaymilliseconds", m_displayMilliseconds);

    settings.setValue("enqueueitems", m_enqueueItems);


    settings.setValue("normalizeenabled", m_normalizeEnabled);
    settings.setValue("normalizefadetime", m_normalizeFadeTime);
    settings.setValue("normalizethreshold", m_normalizeThreshold);
    settings.setValue("normalizemaxamp", m_normalizeMaxAmp);

    settings.setValue("reverbenabled", m_reverbEnabled);
    settings.setValue("reverbpreset", m_reverbPreset);

    settings.setValue("systrayonquitenabled", m_systrayOnQuitEnabled);

    settings.setValue("selectedplaylist", selectedPlaylist);
    settings.setValue("currentplaylist", currentPlaylist);
    settings.setValue("currentRow", currentRow);

    settings.setValue("ignoresuffix", m_ignoreSuffix);
    settings.setValue("ignoreprefix", m_ignorePrefix);

    settings.setValue("colormain", colorMain);
    settings.setValue("colormainhover", colorMainHover);
    settings.setValue("colormedium", colorMedium);
    settings.setValue("colorbackground", colorBackground);
    settings.setValue("colorbehindbackground", colorBehindBackground);
    settings.setValue("colormaintext", colorMainText);
    settings.setValue("colorselection", colorSelection);
    settings.setValue("colorbutton", colorButton);
    settings.setValue("colorbuttonhover", colorButtonHover);
    settings.setValue("colordimmedtext", colorDimmedText);

    settings.setValue("colorvumetertop", colorVisualizerTop);
    settings.setValue("colorvumeterbottom", colorVisualizerBottom);
    settings.setValue("colorvumetermiddle", colorVisualizerMiddle);

    settings.setValue("colorvisualizerbackground", colorVisualizerBackground);

    settings.setValue("colorvumeterpeak", colorVisualizerPeak);
    settings.setValue("vumeterpeaksenabled", vumeterPeaksEnabled);
    settings.setValue("vumeterpeaksheight", vumeterPeaksHeight);

    settings.setValue("playlistrowheight", playlistRowHeight);
    settings.setValue("playlistsrowheight", playlistsRowHeight);
    settings.setValue("nowplayingfontsize", nowPlayingFontSize);

    settings.setValue("vumeterwidth", getEffect()->getVumeterWidth());
    settings.setValue("vumeteropacity", getEffect()->getVumeterOpacity());


    settings.setValue("scroller/amplitude", getEffect()->getAmplitude());
    settings.setValue("scroller/frequency", getEffect()->getFrequency());
    settings.setValue("scroller/sinusspeed", getEffect()->getSinusSpeed());
    settings.setValue("scroller/scrollspeed", getEffect()->getScrollSpeed());
    settings.setValue("scroller/verticalscrollposition", getEffect()->getVerticalScrollPosition());
    settings.setValue("scroller/fontscaleX", getEffect()->getFontScaleX());
    settings.setValue("scroller/fontscaleY", getEffect()->getFontScaleY());
    settings.setValue("scroller/reflection", getEffect()->getReflectionEnabled());
    settings.setValue("scroller/customscrolltextenabled", getEffect()->getCustomScrolltextEnabled());
    settings.setValue("scroller/customscrolltext", getEffect()->getCustomScrolltext());
    settings.setValue("scroller/starfield", getEffect()->getStarsEnabled());
    settings.setValue("scroller/rasterbars", getEffect()->getRasterBarsEnabled());
    settings.setValue("scroller/rasterbarsamount", getEffect()->getNumberOfRasterBars());
    settings.setValue("scroller/rasterbarsspeed", getEffect()->getRasterBarsSpeed());
    settings.setValue("scroller/rasterbarsheight", getEffect()->getRasterBarsHeight());
    settings.setValue("scroller/rasterbarsverticalspacing", getEffect()->getRasterBarsVerticalSpacing());
    settings.setValue("scroller/rasterbarsopacity", getEffect()->getRasterbarsOpacity());
    settings.setValue("scroller/numberofstars", getEffect()->getNumberOfStars());
    settings.setValue("scroller/starsdirection", getEffect()->getStarsDirection());
    settings.setValue("scroller/starspeed", getEffect()->getStarSpeed());
    settings.setValue("scroller/scrollerenabled", getEffect()->getScrollerEnabled());
    settings.setValue("scroller/printerenabled", getEffect()->getPrinterEnabled());
    settings.setValue("scroller/vumeterenabled", getEffect()->getVUMeterEnabled());
    settings.setValue("scroller/reflectionColor", getEffect()->getReflectionColor());
    settings.setValue("scroller/font", getEffect()->getFont());
    settings.setValue("reflectionopacity", getEffect()->getReflectionOpacity());
    settings.setValue("scroller/sinusfontscaling", getEffect()->getSinusFontScalingEnabled());

    settings.setValue("visualizer/keepaspectratio", getEffect()->getKeepAspectRatio());
    settings.setValue("visualizer/resolutionwidth", getEffect()->getResolutionWidth());
    settings.setValue("visualizer/resolutionheight", getEffect()->getResolutionHeight());


    settings.setValue("printer/font", getEffect()->getPrinterFont());
    settings.setValue("printer/fontscaleX", getEffect()->getPrinterFontScaleX());
    settings.setValue("printer/fontscaleY", getEffect()->getPrinterFontScaleY());

    settings.setValue("samplescolumns/number", sampleColumnNumberWidth);
    settings.setValue("samplescolumns/name", sampleColumnNameWidth);
    settings.setValue("samplescolumns/size", sampleColumnSizeWidth);
    settings.setValue("samplescolumns/loopstart", sampleColumnLoopStartWidth);
    settings.setValue("samplescolumns/loopend", sampleColumnLoopEndWidth);
    settings.setValue("samplescolumns/volume", sampleColumnVolumeWidth);
    settings.setValue("samplescolumns/finetune", sampleColumnFinetuneWidth);
    settings.setValue("samplescolumns/resolution", sampleColumnResolutionWidth);

    settings.setValue("instrumentscolumns/number", instrumentColumnNumberWidth);
    settings.setValue("instrumentscolumns/name", instrumentColumnNameWidth);
    settings.setValue("instrumentscolumns/volume", instrumentColumnVolumeWidth);
    settings.setValue("instrumentscolumns/wavelength", instrumentColumnWaveLengthWidth);

    settings.setValue("infocolumns/name", infoNameWidth);
    settings.setValue("infocolumns/value", infoValueWidth);


    settings.setValue("AllowOnlyOneInstance", isOnlyOneInstanceEnabled());

    settings.setValue("lastOpenedDir", lastDir);

    savePlayListSettings();

    if (PLUGIN_libsidplayfp_LIB != "")
    {
        settings.setValue(QString("libsidplayfp/updateFrequency"), HvscSonglengthsFrequency);
    }
}

void MainWindow::savePlayListSettings()
{
    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    //Clear old playlist settings
    settings.remove("playlists");

    //Iterate all current playlists and save the order and the column settings
    QStringList orderedKeys;
    settings.beginGroup("playlists");
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        orderedKeys << ui->listWidget->item(i)->text();
        QListWidgetItem* item = ui->listWidget->item(i);
        QString key = item->text();
        if (tableWidgetPlaylists.contains(key)) {
            QTableView* view = tableWidgetPlaylists.value(key);
            settings.setValue(key, view->horizontalHeader()->saveState());
        }
    }
    settings.endGroup();
    settings.setValue("playlistOrder", orderedKeys);

}

vector<PlaylistItem*> MainWindow::getPlayListEntriesM3U(QString filename)
{
    std::vector<PlaylistItem*> entries;
    QFile file(filename);
    //    Song song;
    //    song.startTime = 0;
    //    song.length = 0;
    //    song.title = "";
    //    song.startSubsong = 0;
    //    song.startSubsongPlayList = -1;

    QString extInfoTitle = "";
    QString extInfoLength = "";

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        QString line = "";
        while (!line.isNull())
        {
            PlaylistItem* playlistItem = new PlaylistItem();

            line = in.readLine();
            if (line.startsWith("#EXTINF", Qt::CaseInsensitive))
            {
                //we got some extrainfo (a title), we save it for the next file
                int i = line.indexOf(',');
                if (i == -1) //invalid format, but we show everything except "#EXTINF"
                {
                    i = 7;
                }
                extInfoTitle = line.mid(i + 1).trimmed();
                extInfoLength = line.mid(8, i - 8).trimmed();
                //addDebugText("title from m3u: "+ extInfoTitle);
            }
            else if (!line.startsWith("#EXTM3U", Qt::CaseInsensitive) && !line.startsWith(
                "#EXTINF", Qt::CaseInsensitive))
            {
                QStringList NEZlist = line.split(NEZPLAYLISTSPLITTER);

                QStringList NEZParameterList;
                if (NEZlist.size() > 1)
                {
                    //Has extended NEZ info
                    line = NEZlist.at(0);
                    NEZParameterList = NEZlist.at(1).split(PLAYLISTFIELDSPLITTER);
                }
                QDir fileInPlayList(line);
                if (fileInPlayList.isAbsolute())
                {
                    //replace all \ with /
                    line = line.replace('\\', '/');
                    int i = line.lastIndexOf('/');
                    QString onlyFilename = line.mid(i + 1);
                    playlistItem->filename = onlyFilename.trimmed();
                    playlistItem->fullPath = QString(line).trimmed();
                    playlistItem->title = playlistItem->filename;
                }
                else //it's a relative path
                {
                    //url?
                    if (line.startsWith("file:///", Qt::CaseInsensitive))
                    {
                        QUrl url(line);
                        line = url.toLocalFile();
                        //addDebugText("url: " + line);
                        playlistItem->filename = line.trimmed();
                        playlistItem->fullPath = line.trimmed();
                        playlistItem->title = playlistItem->filename;
                    }
                    else if (line.startsWith("file://", Qt::CaseInsensitive))
                    {
                        QUrl url(line);
                        line = url.toLocalFile();
                        addDebugText("url: " + line);
                        playlistItem->filename = line.trimmed();
                        playlistItem->fullPath = line.trimmed();
                        playlistItem->title = playlistItem->filename;
                    }
                    //if it's a web location, don't add the playlist path
                    else if (!line.startsWith("http://", Qt::CaseInsensitive) && !line.startsWith(
                        "https://", Qt::CaseInsensitive))
                    {
                        //get the path of the playlist file
                        int i = filename.lastIndexOf('/');
                        QString playlistpath = filename.left(i);
                        QString completepath = playlistpath + "/" + line;
                        completepath = completepath.replace('\\', '/');
                        i = completepath.lastIndexOf('/');
                        QString onlyFilename = completepath.mid(i + 1);
                        playlistItem->filename = onlyFilename.trimmed();
                        playlistItem->title = playlistItem->filename;
                        playlistItem->fullPath = QString(completepath).trimmed();
                    }
                    else
                    {
                        playlistItem->filename = line.trimmed();
                        playlistItem->fullPath = line.trimmed();
                        playlistItem->title = playlistItem->filename;
                    }
                }
                playlistItem->fullPath = QUrl::fromPercentEncoding(playlistItem->fullPath.toStdString().c_str());
                playlistItem->subsongs = 1;
                //check if we have extrainfo
                if (extInfoTitle != "")
                {
                    playlistItem->title = extInfoTitle;
                    extInfoTitle = "";
                }

                playlistItem->length = -1;
                if (extInfoLength != "")
                {
                    if (extInfoLength != "-1" && extInfoLength != "0")
                    {
                        playlistItem->length = (extInfoLength.toInt()) * 1000;
                    }
                    else
                    {
                        playlistItem->length = -1;
                    }
                    extInfoLength = "";
                }


                //check if we have NEZ extrainfo
                if (NEZParameterList.size() > (3))
                {
                    if (!NEZParameterList.at(2).isEmpty())
                    {
                        playlistItem->title = NEZParameterList.at(2);
                    }

                    QStringList timelist = NEZParameterList.at(3).split(":");


                    float seconds = 0;
                    int minutes = 0;
                    int hours = 0;
                    if (timelist.size() == 3)
                    {
                        hours = timelist.at(0).toInt() * 3600;
                        minutes = timelist.at(1).toInt() * 60;
                        seconds = timelist.at(2).toFloat();
                    }
                    if (timelist.size() == 2)
                    {
                        minutes = timelist.at(0).toInt() * 60;
                        seconds = timelist.at(1).toFloat();
                    }
                    if (timelist.size() == 1)
                    {
                        seconds = timelist.at(0).toFloat();
                    }

                    if (hours + minutes + seconds > 0)
                    {
                        playlistItem->length = (hours + minutes + seconds) * 1000;
                    }
                    bool subsongOK = false;
                    if (NEZParameterList.at(1).startsWith("$"))
                    {
                        QString strStartSubsong = NEZParameterList.at(1).mid(1);
                        playlistItem->startSubsongPlayList = strStartSubsong.toInt(&subsongOK, 16);
                    }
                    else
                    {
                        playlistItem->startSubsongPlayList = NEZParameterList.at(1).toInt(&subsongOK);
                    }

                    if (!subsongOK)
                    {
                        playlistItem->startSubsongPlayList = -1;
                    }

                    playlistItem->fileFormat = NEZParameterList.at(0);

                    if (NEZParameterList.size() > (7))
                    {
                        playlistItem->artist = NEZParameterList.at(7);
                    }
                }

                if (!playlistItem->filename.isEmpty())
                {
                    QFile file(playlistItem->filename);
                    QFileInfo fileInfo(file.fileName());
                    QString filename(fileInfo.fileName());

                    QStringList ignorePrefixList = getIgnorePrefix().split(";");
                    QStringListIterator itIgnorePrefixFiles(ignorePrefixList);
                    QStringList ignoreSuffixList = getIgnoreSuffix().split(";");
                    QStringListIterator itIgnoreSuffixFiles(ignoreSuffixList);
                    bool ignoreThisFile = false;
                    while (itIgnorePrefixFiles.hasNext())
                    {
                        QString ignoreFilePrefix = itIgnorePrefixFiles.next() + ".";
                        if (filename.startsWith(ignoreFilePrefix, Qt::CaseInsensitive))
                        {
                            ignoreThisFile = true;
                        }
                    }


                    while (itIgnoreSuffixFiles.hasNext())
                    {
                        QString ignoreFileSuffix = "." + itIgnoreSuffixFiles.next();
                        if (filename.endsWith(ignoreFileSuffix, Qt::CaseInsensitive))
                        {
                            ignoreThisFile = true;
                        }
                    }

                    if (!ignoreThisFile)
                    {
                        entries.push_back(playlistItem);
                    }
                }
            }
        }
    }
    return entries;
}

QString MainWindow::createPlaylist(QString name)
{
    QString newFilename;
    QString newOrgFilename;
    newOrgFilename = name;
    QListWidgetItem* newItem = new QListWidgetItem;


    newFilename = newOrgFilename;
    int suffix = 0;


    while (tableWidgetPlaylists.contains(newFilename))
    {
        suffix++;
        newFilename = newOrgFilename + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
    }

    newItem->setText(newFilename);
    newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
    ui->listWidget->insertItem(ui->listWidget->count(), newItem);
    MyItemDelegate* item = new MyItemDelegate(this);
    item->setMainColor(QColor(colorMain.left(7)));
    ui->listWidget->setItemDelegate(item);




    QFont roboto("Roboto");


    DraggableTableView* tv = new DraggableTableView();
    tv->setDragBackgroundColor(QColor(colorMain.left(7)));
    tv->setDragTextColor(QColor(colorMainText.left(7)));
    tv->setupDelegate(); // has to be called after colors are set

    PlaylistModel* pm = new PlaylistModel(this);
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(pm); // create proxy
    proxyModel->setSourceModel(pm);
    tv->setModel(proxyModel);
    tv->setColumnHidden(4, true);
    tv->setColumnHidden(5, true);
    tv->setColumnHidden(6, true);
    tv->setColumnHidden(7, true);
    tableWidgetPlaylists[newItem->text()] = tv;

    tableWidgetPlaylists[newItem->text()]->setStyleSheet(
        ui->dockWidgetContents_4->styleSheet() + "QHeaderView::section{font-family:Roboto;} QTableView{padding:9px;}");
    tableWidgetPlaylists[newItem->text()]->setFont(roboto);
    tableWidgetPlaylists[newItem->text()]->installEventFilter(this);

    createThePopupMenuCurrentPlaylist(newItem->text());

    connect(tableWidgetPlaylists[newItem->text()], SIGNAL(doubleClicked(const QModelIndex &)),
            SLOT(on_playlist_itemDoubleClicked(const QModelIndex &)));

    swapColumns(tableWidgetPlaylists[newItem->text()]);
    return newItem->text();
}

void MainWindow::on_pushButtonNewPlaylist_clicked()
{
    createPlaylist("New Playlist" PLAYLIST_DEFAULT_EXTENSION);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem* item)
{
    selectedPlaylist = item->text();
    DockWidgetPlaylist->setWindowTitle(item->text());
    DockWidgetPlaylist->setWidget(tableWidgetPlaylists[item->text()]);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    //change playlist
    //play first song

    //Remove highlighted playlist
    //There might by a state where no playlist is highlighted if a user
    //Removed a playing playlist
    QList<QListWidgetItem*> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
    if (l.size() > 0)
    {
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
    }

    removeHighlight();
    currentPlaylist = ui->listWidget->currentItem()->text();
    if (isShuffleEnabled())
    {
        resetShuffle(currentPlaylist);
    }
    removeHighlight();
    currentRow = 0;

    PlaySong(currentRow);
}
bool MainWindow::isShuffleEnabled() const
{
    return ui->checkBoxShuffle->checkState() == Qt::Checked;
}
void MainWindow::iffWriteChunkHeader(FILE* f, char* chunkName, uint32_t chunkLen)
{
    fwrite(chunkName, sizeof(int32_t), 1, f);
    chunkLen = SWAP32(chunkLen);
    fwrite(&chunkLen, sizeof(int32_t), 1, f);
}


void MainWindow::iffWriteUint32(FILE* f, uint32_t value)
{
    value = SWAP32(value);
    fwrite(&value, sizeof(int32_t), 1, f);
}

void MainWindow::iffWriteUint16(FILE* f, uint16_t value)
{
    value = SWAP16(value);
    fwrite(&value, sizeof(int16_t), 1, f);
}

void MainWindow::iffWriteUint8(FILE* f, const uint8_t value)
{
    fwrite(&value, sizeof(int8_t), 1, f);
}

void MainWindow::iffWriteChunkData(FILE* f, const void* data, size_t length)
{
    fwrite(data, sizeof(int8_t), length, f);
    if (length & 1) fputc(0, f); // write pad byte if chunk size is uneven
}


void MainWindow::on_checkBoxShuffle_clicked()
{
    if (isShuffleEnabled())
    {
        resetShuffle(currentPlaylist);
        ui->checkBoxShuffle->setToolTip(("Disable shuffle"));
    }
    else
    {
        ui->checkBoxShuffle->setToolTip(("Enable shuffle"));
    }
    updateButtons();
}

void MainWindow::on_checkBoxLoop_clicked()
{
    if (ui->checkBoxLoop->checkState() == Qt::Unchecked)
    {
        Playmode = normal;
        ui->checkBoxLoop->setToolTip(("Enable repeat"));
    }
    else if (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked)
    {
        Playmode = repeatPlaylist;
        ui->checkBoxLoop->setToolTip(("Enable repeat one"));
    }
    else
    {
        ui->checkBoxLoop->setToolTip(("Disable repeat"));
        Playmode = repeatSong;
    }
    updateButtons();
}

void MainWindow::clickSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        restoreFromTray();
    }
}

void MainWindow::restoreFromTray()
{
    //Restore from system tray
    if (this->isMinimized())
    {
        if (wasMaxmimized)
        {
            showMaximized();
        }
        else
        {
            showNormal();
        }

        activateWindow();
        if (!this->isVisible())
        {
            this->setWindowTitle(windowTitle);
        }
        else
        {
            this->setWindowTitle(PROJECT_NAME_VERSIONED);
        }
    }
    //minimize to system tray (user clicked on system tray)
    else if (this->isVisible())
    {
        if (isMaximized())
        {
            wasMaxmimized = true;
        }
        else
        {
            wasMaxmimized = false;
        }
        hide();
        if (m_Tray->isVisible())
        {
            m_Tray->setToolTip(windowTitle);
        }
    }
    //Restore from system tray but when we clicked system tray (and not minimized window)
    else
    {
        show();
        activateWindow();
        if (!this->isVisible())
        {
            this->setWindowTitle(windowTitle);
        }
        else
        {
            this->setWindowTitle(PROJECT_NAME_VERSIONED);
        }
    }
}

void MainWindow::quit()
{
    playStarted = false;
    SoundManager::getInstance().Stop();
    SoundManager::getInstance().ShutDown();


    QMap<QString, QList<PlaylistItem*>>::iterator i;

    for (auto e: tableWidgetPlaylists.keys()) {
        savePlayList(userPath + PLAYLISTS_DIR + "/" + e,
                     userPath + PLAYLISTS_DIR + "/" + e);
    }

    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("dockingState", m_DockManager->saveState());
    QApplication::quit();
}

void MainWindow::setColorMain(QString mainColor)
{
    mainColor = mainColor + "/*main*/";
    this->colorMainOld = this->colorMain;
    this->colorMain = mainColor;
    changeStyleSheetColor();
}

void MainWindow::setColorMainHover(QString mainColorHover)
{
    mainColorHover = mainColorHover + "/*main hover*/";
    this->colorMainHoverOld = this->colorMainHover;
    this->colorMainHover = mainColorHover;
    changeStyleSheetColor();
}

void MainWindow::setColorMedium(QString mediumColor)
{
    mediumColor = mediumColor + "/*medium*/";
    this->colorMediumOld = this->colorMedium;
    this->colorMedium = mediumColor;
    changeStyleSheetColor();
}

void MainWindow::setColorBackground(QString backgroundColor)
{
    backgroundColor = backgroundColor + "/*background*/";
    this->colorBackgroundOld = this->colorBackground;
    this->colorBackground = backgroundColor;
    changeStyleSheetColor();
}

void MainWindow::setColorBehindBackground(QString behindBackgroundColor)
{
    behindBackgroundColor = behindBackgroundColor + "/*behind-background*/";
    this->colorBehindBackgroundOld = this->colorBehindBackground;
    this->colorBehindBackground = behindBackgroundColor;
    changeStyleSheetColor();
}

void MainWindow::setColorMainText(QString mainTextColor)
{
    mainTextColor = mainTextColor + "/*main text*/";
    this->colorMainTextOld = this->colorMainText;
    this->colorMainText = mainTextColor;
    changeStyleSheetColor();
}

void MainWindow::setColorSelection(QString selectionColor)
{
    selectionColor = selectionColor + "/*selection*/";
    this->colorSelectionOld = this->colorSelection;
    this->colorSelection = selectionColor;
    changeStyleSheetColor();
}

void MainWindow::setColorButton(QString buttonColor)
{
    buttonColor = buttonColor + "/*button*/";
    this->colorButtonOld = this->colorButton;
    this->colorButton = buttonColor;
    changeStyleSheetColor();
}

void MainWindow::setColorButtonHover(QString buttonColorHover)
{
    buttonColorHover = buttonColorHover + "/*button hover*/";
    this->colorButtonHoverOld = this->colorButtonHover;
    this->colorButtonHover = buttonColorHover;
    changeStyleSheetColor();
}

void MainWindow::setColorDimmedText(QString dimmedTextColor)
{
    dimmedTextColor = dimmedTextColor + "/*dimmed text*/";
    this->colorDimmedTextOld = this->colorDimmedText;
    this->colorDimmedText = dimmedTextColor;
    changeStyleSheetColor();
}

void MainWindow::setColorVisualizerTop(const QString& newColor)
{
    this->colorVisualizerTop = newColor;
    ui->visualizer->getEffect()->setColorVisualizerTop(QColor(newColor));
}

void MainWindow::setColorVisualizerBottom(const QString& newColor)
{
    this->colorVisualizerBottom = newColor;
    ui->visualizer->getEffect()->setColorVisualizerBottom(QColor(newColor));
}

void MainWindow::setColorVisualizerPeakColor(const QString& newColor)
{
    this->colorVisualizerPeak = newColor;
    ui->visualizer->getEffect()->setColorVisualizerPeakColor(newColor);
}

void MainWindow::setVUMeterPeaksEnabled(bool enabled)
{
    this->vumeterPeaksEnabled = enabled;
    ui->visualizer->getEffect()->setVUMeterPeaksEnabled(enabled);
}

void MainWindow::setVUMeterPeaksHeight(int height)
{
    this->vumeterPeaksHeight = height;
    ui->visualizer->getEffect()->setHeightVisualizerPeak(height);
}

void MainWindow::setColorVisualizerMiddle(const QString& newColor)
{
    this->colorVisualizerMiddle = newColor;
    ui->visualizer->getEffect()->setColorVisualizerMiddle(QColor(newColor));
}

void MainWindow::setColorVisualizerBackground(const QString& newColor)
{
    this->colorVisualizerBackground = newColor;
    ui->visualizer->getEffect()->setColorVisualizerBackground(QColor(newColor));
}

Effect* MainWindow::getEffect() const
{
    return ui->visualizer->getEffect();
}

void MainWindow::setNowPlayingFontSize(int fontSize)
{
    nowPlayingFontSize = fontSize;
    QFont font = ui->labelFilename->font();
    font.setPixelSize(nowPlayingFontSize);
    ui->labelFilename->setFont(font);
}

void MainWindow::setPlaylistRowHeight(int newRowHeight)
{
    playlistRowHeight = newRowHeight;

    QMapIterator<QString, QTableView*> i(tableWidgetPlaylists);

    while (i.hasNext())
    {
        i.next();
        tableWidgetPlaylists[i.key()]->verticalHeader()->setDefaultSectionSize(playlistRowHeight);
    }
}

void MainWindow::setPlaylistsRowHeight(int newRowHeight)
{
    playlistsRowHeight = newRowHeight;

    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        QListWidgetItem* item = ui->listWidget->item(i);
        item->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
    }
}

void MainWindow::createTrayMenu()
{
    //trayicon----------------------------------------

    m_Tray = new QSystemTrayIcon(this);
    QIcon trayIcon;

    trayIcon = QIcon(":/static/data/resources/icon.png");
    m_Tray->setIcon(trayIcon);
    m_Tray->setToolTip(PROJECT_NAME);

    connect(m_Tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(clickSysTrayIcon(QSystemTrayIcon::ActivationReason)));

    addFilesAction = new QAction("Add File(s)", this);
    connect(addFilesAction, SIGNAL(triggered()), this, SLOT(addFiles()));

    playAction = new QAction("Play", this);
    connect(playAction, SIGNAL(triggered()), this, SLOT(on_buttonPlay_2_clicked()));

    nextAction = new QAction("Next", this);
    connect(nextAction, SIGNAL(triggered()), this, SLOT(on_buttonNext_clicked()));

    prevAction = new QAction("Previous", this);
    connect(prevAction, SIGNAL(triggered()), this, SLOT(on_buttonPrev_clicked()));

    muteAction = new QAction("Mute", this);
    connect(muteAction, SIGNAL(triggered()), this, SLOT(muteVolume()));

    quitAction = new QAction("Quit", this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(addFilesAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(muteAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_Tray->setContextMenu(trayIconMenu);
}

void MainWindow::on_checkBoxVolumeOn_clicked()
{
    bool mute = ui->checkBoxVolumeOn->checkState() != Qt::Checked;
    QString strMute = mute ? "true" : "false";
    SoundManager::getInstance().SetMute(mute);
    m_muteVolume = mute;
    if (m_muteVolume)
    {
        muteAction->setText("Unmute");
    }
    else
    {
        muteAction->setText("Mute");
    }
    addDebugText("Mute volume: " + strMute);
    updateButtons();
}

void MainWindow::muteVolume()
{
    m_muteVolume = !m_muteVolume;
    ui->checkBoxVolumeOn->setChecked(!m_muteVolume);

    QString strMute = m_muteVolume ? "true" : "false";
    muteAction->setText("Unmute");
    if (m_muteVolume)
    {
        muteAction->setText("Unmute");
    }
    else
    {
        muteAction->setText("Mute");
    }
    SoundManager::getInstance().SetMute(m_muteVolume);
    updateButtons();
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    int vol = value;
    SoundManager::getInstance().SetVolume((float)vol / 100);
    ui->checkBoxVolumeOn->setCheckState(Qt::Checked);
    SoundManager::getInstance().SetMute(false);
    m_muteVolume = false;
}

void MainWindow::on_playlist_doubleClicked(const QModelIndex& index)
{
    addDebugText(" dbl clicked");
}


void MainWindow::on_actionRestore_Default_triggered()
{
    restoreLayout();
}

void MainWindow::on_actionNew_Workspace_triggered()
{
    DialogNewWorkspace dialogNewWorkspace(this);
    dialogNewWorkspace.exec();
}

void MainWindow::on_actionDelete_Workspace_triggered()
{
    DialogDeleteWorkspace dialogDeleteWorkspace(this);
    dialogDeleteWorkspace.exec();
}

void MainWindow::on_actionPreferences_triggered()
{
    openSettings();
}


void MainWindow::on_actionAdd_folder_triggered()
{
    addFolder();
}


void MainWindow::on_actionAdd_file_s_triggered()
{
    addFiles();
}

void MainWindow::on_actionAdd_network_stream_triggered()
{
    bool ok;
    QString url = QInputDialog::getText(this, tr("Add URL"),
                                        tr("Enter URL to open:\nFor example: http://www.myfavradio.com:80/stream/1234"),
                                        QLineEdit::Normal,
                                        0, &ok);
    if (ok && !url.isEmpty())
    {
        QList<QUrl> list;
        list.append(QUrl(url));
        addSong(list, 0, selectedPlaylist, false);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    quit();
}

void MainWindow::on_actionAbout_BZR_Player_triggered()
{
    about about(this);
    about.exec();
}

void MainWindow::on_pitchSlider_valueChanged(int value)
{
    SoundManager::getInstance().SetFrequencyByMultiplier(ui->pitchSlider->value() / 100.0);
    ui->labelPitchValue->setText(QString::number(ui->pitchSlider->value()) + " %");
}


void MainWindow::on_positionSlider_valueChanged(int value)
{
    QString timeToShow = QString(msToNiceStringExact(value, m_displayMilliseconds));
    ui->labelTimer_2->setText(timeToShow);
}

void MainWindow::setFmodSeamlessLoopEnabled(bool seamlessLoop) {
    isFmodSeamlessLoopEnabled = seamlessLoop;
}

bool MainWindow::getFmodSeamlessLoopEnabled() {
    return isFmodSeamlessLoopEnabled;
}

const QString& MainWindow::getColorMain() const
{
    return colorMain;
}

const QString& MainWindow::getColorMainHover() const
{
    return colorMainHover;
}

const QString& MainWindow::getColorMedium() const
{
    return colorMedium;
}

const QString& MainWindow::getColorBackground() const
{
    return colorBackground;
}

const QString& MainWindow::getColorBehindBackground() const
{
    return colorBehindBackground;
}

const QString& MainWindow::getColorMainText() const
{
    return colorMainText;
}

const QString& MainWindow::getColorSelection() const
{
    return colorSelection;
}

const QString& MainWindow::getColorButton() const
{
    return colorButton;
}

const QString& MainWindow::getColorButtonHover() const
{
    return colorButtonHover;
}

const QString& MainWindow::getColorDimmedText() const
{
    return colorDimmedText;
}

const QString& MainWindow::getColorVisualizerTop() const
{
    return colorVisualizerTop;
}

const QString& MainWindow::getColorVisualizerBottom() const
{
    return colorVisualizerBottom;
}

const QString& MainWindow::getColorVisualizerMiddle() const
{
    return colorVisualizerMiddle;
}

const QString& MainWindow::getColorVisualizerBackground() const
{
    return colorVisualizerBackground;
}

const QString& MainWindow::getColorVisualizerPeakColor() const
{
    return colorVisualizerPeak;
}


void MainWindow::changeStyleSheetColor()
{
    ui->visualizer->setBackgroundColor(colorBackground.left(7));
    ui->trackerView->setBackgroundColor(colorBackground.left(7));
    QString stylesheet;

    QMapIterator<QString, QTableView*> i(tableWidgetPlaylists);

    while (i.hasNext())
    {
        i.next();
        stylesheet = i.value()->styleSheet();
        stylesheet.replace(colorBackgroundOld, colorBackground);
        stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
        stylesheet.replace(colorMainOld, colorMain);
        stylesheet.replace(colorMediumOld, colorMedium);
        stylesheet.replace(colorMainTextOld, colorMainText);
        stylesheet.replace(colorSelectionOld, colorSelection);
        stylesheet.replace(colorButtonOld, colorButton);
        stylesheet.replace(colorButtonHoverOld, colorButtonHover);
        stylesheet.replace(colorDimmedTextOld, colorDimmedText);
        i.value()->setStyleSheet(stylesheet);
    }


    stylesheet = ui->positionSlider->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->positionSlider->setStyleSheet(stylesheet);

    stylesheet = ui->pitchSlider->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->pitchSlider->setStyleSheet(stylesheet);

    stylesheet = ui->volumeSlider->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->volumeSlider->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_7->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->dockWidgetContents_7->setStyleSheet(stylesheet);


    stylesheet = ui->checkBoxShuffle->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->checkBoxShuffle->setStyleSheet(stylesheet);

    stylesheet = ui->checkBoxLoop->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->checkBoxLoop->setStyleSheet(stylesheet);

    stylesheet = ui->bottom_2->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->bottom_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_7->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_7->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_5->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_5->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_3->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_3->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_2->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_10->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_10->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_9->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_9->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_4->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_4->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_8->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_8->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_6->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_6->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_11->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents_11->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidget->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidget->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetContents->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetChannels->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetChannels->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetFilename->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetFilename->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetVisualizer->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetVisualizer->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetInfo->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetInfo->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetSamples->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetSamples->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetInstruments->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetInstruments->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetLogger->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetLogger->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetPlaylist->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetPlaylist->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetPlaylists->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetPlaylists->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetTrackerView->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->dockWidgetTrackerView->setStyleSheet(stylesheet);


    stylesheet = this->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    this->setStyleSheet(stylesheet);

    stylesheet = ui->buttonPlay_2->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->buttonPlay_2->setStyleSheet(stylesheet);

    stylesheet = this->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    this->setStyleSheet(stylesheet);

    stylesheet = this->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    this->setStyleSheet(stylesheet);

    stylesheet = this->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    this->setStyleSheet(stylesheet);

    stylesheet = this->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    this->setStyleSheet(stylesheet);

    stylesheet = this->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    this->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_2->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    ui->dockWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_10->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    ui->dockWidgetContents_10->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_9->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    ui->dockWidgetContents_9->setStyleSheet(stylesheet);

    stylesheet = ui->pitchSlider->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->pitchSlider->setStyleSheet(stylesheet);

    stylesheet = ui->volumeSlider->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->volumeSlider->setStyleSheet(stylesheet);

    stylesheet = ui->positionSlider->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->positionSlider->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_2->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_10->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_10->setStyleSheet(stylesheet);


    stylesheet = ui->dockWidgetContents_3->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_3->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_4->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_4->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_8->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_8->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_9->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_9->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_7->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    ui->dockWidgetContents_7->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_2->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_10->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_10->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_9->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_9->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_4->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_4->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_8->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_8->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_3->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_3->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_5->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_5->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_7->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->dockWidgetContents_7->setStyleSheet(stylesheet);

    stylesheet = ui->labelFilename->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    ui->labelFilename->setStyleSheet(stylesheet);


    stylesheet = ui->dockWidgetContents_2->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_10->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_10->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_9->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_9->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_4->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_4->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_8->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_8->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_3->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_3->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_5->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_5->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetContents_7->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->dockWidgetContents_7->setStyleSheet(stylesheet);

    stylesheet = ui->labelFilename->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->labelFilename->setStyleSheet(stylesheet);

    stylesheet = ui->checkBoxVolumeOn->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->checkBoxVolumeOn->setStyleSheet(stylesheet);

    stylesheet = ui->buttonPlay_2->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->buttonPlay_2->setStyleSheet(stylesheet);

    stylesheet = ui->buttonPrev->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->buttonPrev->setStyleSheet(stylesheet);

    stylesheet = ui->buttonNext->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->buttonNext->setStyleSheet(stylesheet);

    stylesheet = ui->buttonStop->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->buttonStop->setStyleSheet(stylesheet);


    stylesheet = ui->checkBoxShuffle->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->checkBoxShuffle->setStyleSheet(stylesheet);

    stylesheet = ui->checkBoxLoop->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->checkBoxLoop->setStyleSheet(stylesheet);

    stylesheet = ui->pushButtonNewPlaylist->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->pushButtonNewPlaylist->setStyleSheet(stylesheet);

    stylesheet = ui->labelTimer_2->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->labelTimer_2->setStyleSheet(stylesheet);

    stylesheet = ui->labelFileLength_2->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->labelFileLength_2->setStyleSheet(stylesheet);

    stylesheet = ui->labelPitch->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->labelPitch->setStyleSheet(stylesheet);

    stylesheet = ui->labelPitchValue->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->labelPitchValue->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidget->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidget->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetChannels->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetChannels->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetFilename->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetFilename->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetVisualizer->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetVisualizer->setStyleSheet(stylesheet);


    stylesheet = ui->dockWidgetInfo->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetInfo->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetSamples->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetSamples->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetInstruments->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetInstruments->setStyleSheet(stylesheet);


    stylesheet = ui->dockWidgetLogger->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetLogger->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetPlaylist->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetPlaylist->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetPlaylists->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetPlaylists->setStyleSheet(stylesheet);

    stylesheet = ui->dockWidgetTrackerView->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->dockWidgetTrackerView->setStyleSheet(stylesheet);

    stylesheet = ui->menuBar->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    ui->menuBar->setStyleSheet(stylesheet);

    stylesheet = ui->menuBar->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    ui->menuBar->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = m_DockManager->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    m_DockManager->setStyleSheet(stylesheet);

    stylesheet = ui->listWidget->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->listWidget->setStyleSheet(stylesheet);

    settingsWindow settingsWindow(this);
    settingsWindow.changeStyleSheetColor();
}

void MainWindow::on_buttonStop_clicked() {
    resetAll();
    SoundManager::getInstance().ShutDown();

    if (SoundManager::getInstance().isWavWriterDeviceSelected()) {
        SoundManager::getInstance().Init(FMOD_OUTPUTTYPE_NOSOUND, ""); //Set sound device to silent
    }
}

void MainWindow::resetAll()
{
    SoundManager::getInstance().Stop();
    //ui->visualizer->stop();
    playAction->setText("Play");
    SoundManager::getInstance().Release();
    if (SoundManager::getInstance().m_Info1 != nullptr)
    {
        SoundManager::getInstance().m_Info1->clear();
    }
    playStarted = false;
    loaded = false;
    //Clear all windows/text etc.

    updateInstruments();
    ui->tableInfo->clearContents();
    ui->tableInfo->setRowCount(0);


    getLength();
    ui->positionSlider->setValue(0);
    if (m_displayMilliseconds)
    {
        ui->labelTimer_2->setText("0:00.000");
    }
    else
    {
        ui->labelTimer_2->setText("0:00");
    }

    ui->labelFilename->clear();
    this->setWindowTitle(PROJECT_NAME_VERSIONED);
    channels->updateChannels();

    removeHighlight();
    updateButtons();
}

void MainWindow::removeHighlight()
{
    if (tableWidgetPlaylists.contains(currentPlaylist))
    {
        QModelIndex index3 = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 7, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index3, false, Qt::EditRole);
        tableWidgetPlaylists[currentPlaylist]->update();
    }
}

bool MainWindow::getVUMeterPeaksEnabled()
{
    return vumeterPeaksEnabled;
}

int MainWindow::getVUMeterPeaksHeight()
{
    return vumeterPeaksHeight;
}

bool MainWindow::isOnlyOneInstanceEnabled() const
{
    return m_bAllowOnlyOneInstanceEnabled;
}

void MainWindow::setAllowOnlyOneInstanceEnabled(bool enabled)
{
    m_bAllowOnlyOneInstanceEnabled = enabled;
}

void MainWindow::setDisplayMilliseconds(bool enabled)
{
    m_displayMilliseconds = enabled;
}

void MainWindow::setEnqueueItems(bool enabled)
{
    m_enqueueItems = enabled;
}

QString MainWindow::getVersion()
{
    return VERSION;
}

void MainWindow::sendSocketMsg()
{
    QStringList args = qApp->arguments();
    args.removeFirst(); //Remove first argument from commandline since it's always "BZRPlayer.exe"
    //addDebugText("Send socket message, count( " + QString::number(args.count()) + ")");
    QByteArray data;
    //    for(int j = 1; j<args.count(); j++)
    //    {
    //        QString fileName = args.at(j);
    //        stringList.append(fileName);
    //    }

    QDataStream dataStreamWrite(&data, QIODevice::WriteOnly);
    dataStreamWrite << args;


    tcpClient->write(data);
    tcpClient->disconnectFromHost();
}

void MainWindow::acceptConnection()
{
    tcpClient = tcpServer->nextPendingConnection();
    connect(tcpClient, SIGNAL(readyRead()), SLOT(getSocketData()));
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    //if (socketError == QTcpSocket::RemoteHostClosedError)
    //    return;

    QMessageBox::information(this, "Network error", "The following error occurred: " + tcpClient->errorString() + ".");
    tcpClient->close();
    tcpServer->close();
}

/*
* Reads all the data from the socket
* the data read is what the instance got from the command line
* if m_bEnqueueFileEnabled is true then we add all sounds to the playlist
* if it is false, we play them directly (only the last one)
*/
void MainWindow::getSocketData()
{
    QByteArray bytes = tcpClient->readAll();

    QDataStream dataStreamRead(bytes);


    QStringList list;
    dataStreamRead >> list;
    //QMessageBox::information(this, "list","count: " + QString::number(list.count()) + ".");
    QList<QUrl> urls;
    for (const auto& item : list)
    {
        urls.append(QUrl().fromLocalFile(item));
    }


    if (list.count() > 0)
    {
        QString command = list.last();
        if (!command.isEmpty())
        {
            if (command.toLower() == "-play")
            {
                //on_buttonPlay_clicked(false,true);
            }
            else if (command.toLower() == "-pause")
            {
                //on_buttonPause_clicked();
            }
            else if (command.toLower() == "-next")
            {
                //on_buttonNext_clicked();
            }
            else if (command.toLower() == "-prev")
            {
                //on_buttonPrev_clicked();
            }
            else if (command.toLower() == "-stop")
            {
                //on_buttonStop_clicked();
            }
            else if (command.toLower() == "-nextsubsound")
            {
                //on_buttonNextSub_clicked();
            }
            else if (command.toLower() == "-prevsubsound")
            {
                //on_buttonPrevSub_clicked();
            }
            else if (command.toLower() == "-clear")
            {
                //m_playListWindow->clear(true);
            }
            else if (command.toLower() == "-quit")
            {
                //close();
            }
            else if (command.toLower().startsWith("-volume[") && command.endsWith("]"))
            {
                //            bool ok = false;
                //            int volume = QString(command.mid(8,command.length()-9)).toInt(&ok);
                //            if(ok)
                //            {
                //                ui.sliderVolume->setValue(volume);
                //            }
            }
            else //no command, just add/play the file
            {
                //            if(m_bEnqueueFileEnabled)
                //            {
                //                m_playListWindow->addSong(QStringList(QStringList(bytes).last()),-1);
                //            }
                //            else
                {
                    //                m_playListWindow->clear(true);
                    //                m_iCurrentSong=0;
                    //                m_playListWindow->addSong(QStringList(QStringList(bytes).last()),-1);
                    //                //play first
                    //                m_bIsStopped = false;
                    //                setPaused(false);
                    //                QFont font = m_playListWindow->ui.tableWidget->item(m_iCurrentSong,0)->font();
                    //                font.setBold(true);
                    //                m_playListWindow->ui.tableWidget->item(m_iCurrentSong,0)->setFont(font);
                    //                m_playListWindow->ui.tableWidget->item(m_iCurrentSong,0)->setTextColor(QColor(25,130,115));
                    //                m_iCurrentSong = 0;
                    //                m_icurrentSubsong = 0;
                    //                ui.sliderProgress->setEnabled(true);
                    //                playSong();

                    const int rowCountBeforeAddSong = tableWidgetPlaylists[currentPlaylist]->model()->rowCount();

                    addDebugText("count: " + QString::number(list.count()));
                    //addSong(list,0,PLAYLIST_DEFAULT_FILENAME,false);
                    addSong(urls, 0, PLAYLIST_DEFAULT_FILENAME, false);

                    if ((m_bAllowOnlyOneInstanceEnabled && !m_enqueueItems) || !m_bAllowOnlyOneInstanceEnabled)
                    {
                        QList<QListWidgetItem*> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
                        l.at(0)->setForeground(QColor(colorMainText.left(7)));
                        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
                        on_listWidget_itemClicked(ui->listWidget->item(0));
                        removeHighlight();
                        currentRow = rowCountBeforeAddSong;
                        PlaySong(currentRow);
                    }
                }
            }
        }
    }
}

QPixmap MainWindow::ChangeSVGColor(QString filename, QString color)
{
    // open svg resource load contents to qbytearray
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray baData = file.readAll();
    // load svg contents to xml document and edit contents
    QDomDocument doc;
    doc.setContent(baData);
    // recurivelly change color
    SetAttrRecur(doc.documentElement(), "path", "rect", "fill", color);
    // create svg renderer with edited contents
    QSvgRenderer svgRenderer(doc.toByteArray());
    // create pixmap target (could be a QImage)
    QPixmap pix(svgRenderer.defaultSize());
    pix.fill(Qt::transparent);
    // create painter to act over pixmap
    QPainter pixPainter(&pix);
    // use renderer to render over painter which paints on pixmap
    svgRenderer.render(&pixPainter);

    return pix;
}


void MainWindow::SetAttrRecur(QDomElement elem, QString strtagname, QString strtagname2, QString strattr,
                              QString strattrval)
{
    // if it has the tagname then overwritte desired attribute
    if (elem.tagName().compare(strtagname) == 0 || elem.tagName().compare(strtagname2) == 0)
    {
        elem.setAttribute(strattr, strattrval);
    }

    // loop all children
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        SetAttrRecur(elem.childNodes().at(i).toElement(), strtagname, strtagname2, strattr, strattrval);
    }
}

void MainWindow::LoadWorkspaces()
{
    QAction* restore = ui->menuRestore_Layout->actions().first();
    workspaceSeparator = ui->menuRestore_Layout->insertSeparator(restore);

    QDir directory(userPath + LAYOUTS_DIR);
    QStringList workspaces = directory.entryList(QStringList() << "*.ini", QDir::Files);
    QAction* action;
    foreach(QString filename, workspaces)
    {
        QFileInfo fileInfo(filename);
        QString basename = fileInfo.baseName();
        action = new QAction(basename);
        ui->menuRestore_Layout->insertAction(workspaceSeparator, action);
        connect(action, &QAction::triggered, this, [&, this, filename] { slot_LoadWorkspace(filename); });
    }
}

void MainWindow::DeleteWorkspace(QString workspace) const
{
    foreach(QAction *action, ui->menuRestore_Layout->actions())
    {
        if (action->isSeparator())
        {
        }
        else if (action->menu())
        {
        }
        else
        {
            if (action->text() == workspace)
            {
                ui->menuRestore_Layout->removeAction(action);
            }
        }
    }
}

void MainWindow::slot_LoadWorkspace(const QString& filename)
{
    QSettings settings(userPath + LAYOUTS_DIR + "/" + filename, QSettings::IniFormat);
    if (!m_DockManager->restoreState(settings.value("dockingState").toByteArray()))
    {
        QMessageBox::critical(this, "Error", "Couldn't load layout.");
    }
}

void MainWindow::CreateNewWorkspace(const QString& filename)
{
    QString fileNameAndExtension = filename + ".ini";
    QSettings settings(userPath + LAYOUTS_DIR + "/" + fileNameAndExtension, QSettings::IniFormat);
    settings.setValue("dockingState", m_DockManager->saveState());
    QAction* action = new QAction(filename);
    ui->menuRestore_Layout->insertAction(workspaceSeparator, action);
    connect(action, &QAction::triggered, this, [&, this, fileNameAndExtension]
    {
        slot_LoadWorkspace(fileNameAndExtension);
    });
}

void MainWindow::downloadHvscSonglengthsComplete()
{
    if (filedownloader->downloadedData().size() > 0)
    {
        QFile file(userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << filedownloader->downloadedData();
            file.close();
            QDateTime::currentDateTime().toSecsSinceEpoch();
            QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
            qint64 seconds = QDateTime::currentDateTime().toSecsSinceEpoch();

            if (getHvscSonglengthsPathDownloaded().compare(
                dataPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH))
            {
                settingsWindow settingsWindow(this);
                settingsWindow.setUiLineEditHvscSonglengthTextForcingRelativePaths(
                    userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH);
                settingsWindow.saveSidplaySettings();
            }

            settings.setValue("libsidplayfp/timehvscsonglengthsdownloaded", seconds);
            settings.setValue("libsidplayfp/hvscsonglengthspath",
                              userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH);
            HvscSonglengthsPathDownloaded = userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH;
            HvscSonglengthsDownloadedEpoch = seconds;
            addDebugText("Downloaded " + filedownloader->getUrl().toString() + " to " + file.fileName());
        }
        else
        {
            addDebugText("Couldn't write to file " + file.fileName());
        }
    }
    else
    {
        addDebugText("Failed to download " + filedownloader->getUrl().toString());
    }
}

void MainWindow::setHvscSonglengthsFrequency(QString freq)
{
    HvscSonglengthsFrequency = freq;
}

//Swaps columns so that artist column is first for default and new playlists
void MainWindow::swapColumns(QTableView* tableview)
{
    tableview->setColumnWidth(0, 200);
    tableview->setColumnWidth(1, 100);
    tableview->setColumnWidth(2, 55);
    tableview->setColumnWidth(3, 75);
    tableview->setColumnWidth(8, 175);

    tableview->horizontalHeader()->swapSections(1, 8);

}

void MainWindow::setupIcons()
{
    QPixmap shuffleOn = ChangeSVGColor(":/resources/shuffle.svg", colorMain.left(7));
    QPixmap shuffleOnHover = ChangeSVGColor(":/resources/shuffle.svg", colorMainHover.left(7));
    QPixmap shuffleOff = ChangeSVGColor(":/resources/shuffle.svg", colorButton.left(7));
    QPixmap shuffleOffHover = ChangeSVGColor(":/resources/shuffle.svg", colorButtonHover.left(7));

    QPixmap speakerOn = ChangeSVGColor(":/resources/speaker.svg", colorButton.left(7));
    QPixmap speakerOnHover = ChangeSVGColor(":/resources/speaker.svg", colorButtonHover.left(7));
    QPixmap speakerOff = ChangeSVGColor(":/resources/speaker-off.svg", colorButton.left(7));
    QPixmap speakerOffHover = ChangeSVGColor(":/resources/speaker-off.svg", colorButtonHover.left(7));


    QPixmap play = ChangeSVGColor(":/resources/play.svg", colorButton.left(7));
    QPixmap playHover = ChangeSVGColor(":/resources/play.svg", colorButtonHover.left(7));
    QPixmap pause = ChangeSVGColor(":/resources/pause.svg", colorButton.left(7));
    QPixmap pauseHover = ChangeSVGColor(":/resources/pause.svg", colorButtonHover.left(7));

    QPixmap stop = ChangeSVGColor(":/resources/stop.svg", colorButton.left(7));
    QPixmap stopHover = ChangeSVGColor(":/resources/stop.svg", colorButtonHover.left(7));

    QPixmap prev = ChangeSVGColor(":/resources/prev.svg", colorButton.left(7));
    QPixmap prevHover = ChangeSVGColor(":/resources/prev.svg", colorButtonHover.left(7));
    QPixmap next = ChangeSVGColor(":/resources/next.svg", colorButton.left(7));
    QPixmap nextHover = ChangeSVGColor(":/resources/next.svg", colorButtonHover.left(7));

    QPixmap repeat = ChangeSVGColor(":/resources/repeat.svg", colorButton.left(7));
    QPixmap repeatHover = ChangeSVGColor(":/resources/repeat.svg", colorButtonHover.left(7));
    QPixmap repeat1 = ChangeSVGColor(":/resources/repeat-1.svg", colorMain.left(7));
    QPixmap repeat1Hover = ChangeSVGColor(":/resources/repeat-1.svg", colorMainHover.left(7));
    QPixmap repeatOn = ChangeSVGColor(":/resources/repeat.svg", colorMain.left(7));
    QPixmap repeatOnHover = ChangeSVGColor(":/resources/repeat.svg", colorMainHover.left(7));

    QPixmap add = ChangeSVGColor(":/resources/add.svg", colorButton.left(7));
    QPixmap addHover = ChangeSVGColor(":/resources/add.svg", colorButtonHover.left(7));

    QPixmap checkBoxOn = ChangeSVGColor(":/resources/checkbox-on.svg", colorMain.left(7));
    QPixmap checkBoxOff = ChangeSVGColor(":/resources/checkbox-off.svg", colorBehindBackground.left(7));
    QPixmap checkBoxOnDisabled = ChangeSVGColor(":/resources/checkbox-on-disabled.svg", colorMedium.left(7));
    QPixmap checkBoxOffDisabled = ChangeSVGColor(":/resources/checkbox-off-disabled.svg", colorMedium.left(7));


    icons["speaker-on"] = speakerOn;
    icons["speaker-off"] = speakerOff;
    icons["speaker-onHover"] = speakerOnHover;
    icons["speaker-offHover"] = speakerOffHover;
    icons["shuffle-on"] = shuffleOn;
    icons["shuffle-onHover"] = shuffleOnHover;
    icons["shuffle-off"] = shuffleOff;
    icons["shuffle-offHover"] = shuffleOffHover;
    icons["play"] = play;
    icons["playHover"] = playHover;
    icons["pause"] = pause;
    icons["pauseHover"] = pauseHover;
    icons["prev"] = prev;
    icons["prevHover"] = prevHover;
    icons["next"] = next;
    icons["nextHover"] = nextHover;
    icons["repeat"] = repeat;
    icons["repeatHover"] = repeatHover;
    icons["repeat-1"] = repeat1;
    icons["repeat-1Hover"] = repeat1Hover;
    icons["repeat-on"] = repeatOn;
    icons["repeat-onHover"] = repeatOnHover;
    icons["stop"] = stop;
    icons["stopHover"] = stopHover;
    icons["add"] = add;
    icons["addHover"] = addHover;
    icons["checkbox-on"] = checkBoxOn;
    icons["checkbox-off"] = checkBoxOff;
    icons["checkbox-on-disabled"] = checkBoxOnDisabled;
    icons["checkbox-off-disabled"] = checkBoxOffDisabled;

    ui->buttonPrev->setIcon(prev);
    ui->buttonNext->setIcon(next);
    ui->buttonStop->setIcon(stop);
    ui->pushButtonNewPlaylist->setIcon(add);
}

void MainWindow::setupAdvancedDockingSystem()
{
    ads::CDockManager::setConfigFlags(ads::CDockManager::DefaultOpaqueConfig);
    ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaHasTabsMenuButton, false);
    ads::CDockManager::setConfigFlag(ads::CDockManager::RetainTabSizeWhenCloseButtonHidden, true);

    m_DockManager = new ads::CDockManager(this);

    m_DockManager->setStyleSheet(
            "QScrollBar:vertical {background-color: #282828/*background*/;     width: 15px;     margin: 0 3px 0 3px;     border: 1px transparent #282828/*background*/;     border-radius: 4px; } QScrollBar::handle:vertical {     background-color: #404040/*medium*/;     min-height: 25px;     border-radius: 4px; } QScrollBar:horizontal {     background-color: #282828/*background*/;     height: 15px;     margin: 3px 0 3px 0;     border: 1px transparent #282828/*background*/;     border-radius: 4px; } QScrollBar::handle:horizontal {     background-color: #404040/*medium*/;     min-width: 5px;     border-radius: 4px; } QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {     background: none; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {     background: none; } QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {     background: none; } QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {     background: none; } QScrollBar::sub-line:vertical {     height: 0;     width: 0; } QScrollBar::add-line:vertical {     height: 0;     width: 0; } QScrollBar::sub-line:horizontal {     height: 0;     width: 0; } QScrollBar::add-line:horizontal {     height: 0;     width: 0; }QScrollArea#dockWidgetScrollArea {	background: #282828/*background*/;padding: 0px;	border: none;} ads--CDockWidgetTab QLabel {font-family:Roboto;font-size:14px;color:#b1b1b1/*dimmed text*/;}ads--CDockWidgetTab[activeTab=\"true\"] QLabel {color:#ffffff/*main text*/;}#tabCloseButton {margin-top: 0;background: none;border: none;}ads--CDockContainerWidget ads--CDockSplitter::handle {background: #161616/*behind-background*/;}ads--CDockAreaTitleBar { background: #282828/*background*/;}ads--CDockWidgetTab {background: #282828/*background*/}");


    DockWidgetLogMessages = new ads::CDockWidget("Log Messages");
    DockWidgetLogMessages->setWidget(ui->dockWidgetContents_9, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetLogMessages);
    dockWidgets.append(DockWidgetLogMessages);

    DockWidgetTitle = new ads::CDockWidget("Now Playing");
    DockWidgetTitle->setWidget(ui->dockWidgetContents_5, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetTitle);
    dockWidgets.append(DockWidgetTitle);


    DockWidgetChannels = new ads::CDockWidget("Channels");
    DockWidgetChannels->setWidget(ui->dockWidgetContents_7);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetChannels);
    dockWidgets.append(DockWidgetChannels);

    DockWidgetTrackerView = new ads::CDockWidget("Tracker View");
    DockWidgetTrackerView->setWidget(ui->dockWidgetContents_6, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetTrackerView);
    dockWidgets.append(DockWidgetTrackerView);

    DockWidgetInfo = new ads::CDockWidget("Info");
    DockWidgetInfo->setWidget(ui->dockWidgetContents_3, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetInfo);
    dockWidgets.append(DockWidgetInfo);


    DockWidgetVisualizer = new ads::CDockWidget("Visualizer");
    DockWidgetVisualizer->setWidget(ui->dockWidgetContents_11, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetVisualizer);
    dockWidgets.append(DockWidgetVisualizer);


    DockWidgetPlaylists = new ads::CDockWidget("Playlists");
    DockWidgetPlaylists->setWidget(ui->dockWidgetContents_8, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::LeftDockWidgetArea, DockWidgetPlaylists);
    dockWidgets.append(DockWidgetPlaylists);

    DockWidgetPlaylist = new ads::CDockWidget("Playlist");
    DockWidgetPlaylist->setWidget(ui->dockWidgetContents_4, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::LeftDockWidgetArea, DockWidgetPlaylist);
    dockWidgets.append(DockWidgetPlaylist);

    DockWidgetInstruments = new ads::CDockWidget("Instruments");
    DockWidgetInstruments->setWidget(ui->dockWidgetContents_10, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::RightDockWidgetArea, DockWidgetInstruments);
    dockWidgets.append(DockWidgetInstruments);

    DockWidgetSamples = new ads::CDockWidget("Samples");
    DockWidgetSamples->setWidget(ui->dockWidgetContents_2, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::RightDockWidgetArea, DockWidgetSamples);
    dockWidgets.append(DockWidgetSamples);

    DockWidgetPlayControl = new ads::CDockWidget("Play Control");
    DockWidgetPlayControl->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
    DockWidgetPlayControl->setWidget(ui->dockWidgetContents, ads::CDockWidget::ForceNoScrollArea);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, DockWidgetPlayControl);
    dockWidgets.append(DockWidgetPlayControl);

}

QStringList MainWindow::sortPreservingOrder(const QStringList& folderPlaylists, const QStringList& sortedPlaylistOrder)
{
    QStringList result;
    QSet<QString> added;

    for (const QString& key : sortedPlaylistOrder) {
        if (folderPlaylists.contains(key)) {
            result.append(key);
            added.insert(key);
        }
    }

    for (const QString& item : folderPlaylists) {
        if (!added.contains(item)) {
            result.append(item);
        }
    }
    return result;
}