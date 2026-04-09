#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QScreen>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QSvgRenderer>
#include <QTimer>
#include "about.h"
#include "dialogdeleteworkspace.h"
#include "dialognewworkspace.h"
#include "DraggableTableView.h"
#include "mainwindow.h"
#include "playlistmodel.h"
#include "plugins.h"
#include "settingswindow.h"
#include "soundmanager.h"
#include "ui_mainwindow.h"
#include "various.h"

#define NEZPLAYLISTSPLITTER "::<>::?<>"
#define PLAYLISTFIELDSPLITTER "<><>::????"
#define PROJECT_NAME "BZR Player"
#define PROJECT_NAME_VERSIONED PROJECT_NAME " " PROJECT_VERSION
#define PLAYLIST_DEFAULT "Default"
#define PLAYLIST_DEFAULT_EXTENSION ".m3u"
#define PLAYLIST_DEFAULT_FILENAME PLAYLIST_DEFAULT PLAYLIST_DEFAULT_EXTENSION

using namespace std;
const QString MainWindow::VERSION = PROJECT_VERSION;

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
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

    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    allowOnlyOneInstance = settings.value("allowOnlyOneInstance", true).toBool();

    qDebug() << "allowOnlyOneInstance: " << allowOnlyOneInstance;

    if (allowOnlyOneInstance) {
        instanceExists = initializeSocket();
    }

    if (instanceExists) return;

    // fonts needs to be added before the GUI
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(dataPath + "/resources" + QDir::separator() + "RobotoMono-Regular.ttf");

    ui->setupUi(this);

    setWindowTitle(PROJECT_NAME_VERSIONED);
    windowTitle = PROJECT_NAME_VERSIONED;
    srand(time(nullptr));

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

    loadWorkspaces();

    createThePopupMenuPlaylists();
    createThePopupMenuInstruments();
    createThePopupMenuChannels();
    createThePopupLogWindow();

    createTrayMenu();

    setSystrayChecked(settings.value("systray", true).toBool());
    isSystrayOnMinimizeChecked = settings.value("minimizeToSystray", false).toBool();
    isSystrayOnMinimizeEnabled = isSystrayChecked;

    setMenuBarHiddenChecked(settings.value("menuBarHidden", false).toBool());

    outputDevice = settings.value("outputDevice").toInt();
    resetVolume = settings.value("defaultAudioLevel", false).toBool();
    resetVolumeValue = settings.value("defaultAudioLevelValue", 100).toInt();

    lastDir = settings.value("Internal/lastOpenedDir", "/").toString();

    defaultPlayMode = settings.value("defaultPlayMode", -1).toInt();
    playMode = static_cast<PlayMode>(defaultPlayMode == -1
                                         ? settings.value("Internal/playMode", 0).toInt()
                                         : defaultPlayMode);

    if (playMode == normal) {
        ui->checkBoxLoop->setCheckState(Qt::Unchecked);
    } else if (playMode == repeatPlaylist) {
        ui->checkBoxLoop->setCheckState(Qt::PartiallyChecked);
    } else {
        ui->checkBoxLoop->setCheckState(Qt::Checked);
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        bundledHvscSonglengthsPath = settings.value("Plugins/libsidplayfpBundledHvscSonglengthsPath",
                                                    dataPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH).toString();
        bundledHvscSonglengthsDownloadEpoch = settings.value("Plugins/libsidplayfpBundledHvscSonglengthsDownloadEpoch",
                                                             0).toLongLong();
        bundledHvscSonglengthsUpdateFrequency = settings.value("Plugins/libsidplayfpBundledHvscSonglengthsUpdate",
                                                               "Weekly").toString();
    }

    ui->checkBoxShuffle->setChecked(settings.value("Internal/shuffle", false).toBool());

    displayMilliseconds = settings.value("displayMilliseconds", false).toBool();

    enqueueItems = settings.value("enqueueItems", false).toBool();

    isShownCheckBoxLoopPoints = settings.value("showLoopPoints", false).toBool();

    normalizeEnabled = settings.value("normalizer", false).toBool();
    normalizeFadeTime = settings.value("normalizerFadeTime", 5000).toInt();
    normalizeThreshold = settings.value("normalizerThreshold", 10).toInt();
    normalizeMaxAmp = settings.value("normalizerMaxAmp", 20).toInt();
    currentPlaylist = settings.value("Internal/currentPlaylist", PLAYLIST_DEFAULT).toString();
    currentRow = settings.value("Internal/currentRow", -1).toInt();
    selectedPlaylist = settings.value("Internal/selectedPlaylist", PLAYLIST_DEFAULT).toString();
    reverbEnabled = settings.value("reverb", false).toBool();
    reverbPreset = settings.value("reverbPreset", "Generic").toString();
    ignoreSuffix = settings.value("ignoreSuffixes", "psflib;psf2lib;dsflib").toString();
    ignorePrefix = settings.value("ignorePrefixes", "").toString();

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

    colorMain = settings.value("Appearance/colorMain", colorMainDefault).toString();
    colorMainHover = settings.value("Appearance/colorMainHover", colorMainHoverDefault).toString();
    colorMedium = settings.value("Appearance/colorMedium", colorMediumDefault).toString();
    colorBackground = settings.value("Appearance/colorBackground", colorBackgroundDefault).toString();
    colorBehindBackground = settings.value("Appearance/colorBehindBackground", colorBehindBackgroundDefault).toString();
    colorMainText = settings.value("Appearance/colorMainText", colorMainTextDefault).toString();
    colorSelection = settings.value("Appearance/colorSelection", colorSelectionDefault).toString();
    colorButton = settings.value("Appearance/colorButton", colorButtonDefault).toString();
    colorButtonHover = settings.value("Appearance/colorButtonHover", colorButtonHoverDefault).toString();
    colorDimmedText = settings.value("Appearance/colorDimmedText", colorDimmedTextDefault).toString();

    colorVisualizerTop = settings.value("Visualizer/vuMeterColorTop", "#ff00dd").toString();
    colorVisualizerMiddle = settings.value("Visualizer/vuMeterColorMiddle", "#0000ff").toString();
    colorVisualizerBottom = settings.value("Visualizer/vuMeterColorBottom", "#00ff5e").toString();
    colorVisualizerBackground = settings.value("Visualizer/colorBackground", "#000000").toString();

    sampleColumnNumberWidth = settings.value("Internal/columnSamplesNumberWidth", 35).toInt();
    sampleColumnNameWidth = settings.value("Internal/columnSamplesNameWidth", 185).toInt();
    sampleColumnSizeWidth = settings.value("Internal/columnSamplesSizeWidth", 70).toInt();
    sampleColumnLoopStartWidth = settings.value("Internal/columnSamplesLoopStartWidth", 70).toInt();
    sampleColumnLoopEndWidth = settings.value("Internal/columnSamplesLoopEndWidth", 70).toInt();
    sampleColumnVolumeWidth = settings.value("Internal/columnSamplesVolumeWidth", 70).toInt();
    sampleColumnFinetuneWidth = settings.value("Internal/columnSamplesFineTuneWidth", 70).toInt();
    sampleColumnResolutionWidth = settings.value("Internal/columnSamplesResolutionWidth", 70).toInt();

    instrumentColumnNumberWidth = settings.value("Internal/columnInstrumentsNumberWidth", 35).toInt();
    instrumentColumnNameWidth = settings.value("Internal/columnInstrumentsNameWidth", 185).toInt();
    instrumentColumnVolumeWidth = settings.value("Internal/columnInstrumentsVolumeWidth", 70).toInt();
    instrumentColumnWaveLengthWidth = settings.value("Internal/columnInstrumentsWaveLengthWidth", 70).toInt();

    infoNameWidth = settings.value("Internal/columnInfoNameWidth", 70).toInt();
    infoValueWidth = settings.value("Internal/columnInfoValueWidth", 70).toInt();

    colorVisualizerPeak = settings.value("Visualizer/vuMeterColorPeak", "#b00e3e").toString();
    vuMeterPeaksEnabled = settings.value("Visualizer/vuMeterPeaks", true).toBool();
    vuMeterPeaksHeight = settings.value("Visualizer/vuMeterPeakHeight", 8).toInt();
    getEffect()->setVuMeterWidth(settings.value("Visualizer/vuMeterWidth", 50.0).toDouble());
    getEffect()->setVuMeterOpacity(settings.value("Visualizer/vuMeterOpacity", 100).toInt());

    getEffect()->setAmplitude(settings.value("Visualizer/scrollerAmplitude", 32).toInt());
    getEffect()->setSinusFrequency(settings.value("Visualizer/scrollerFrequency", 0.0027).toDouble());
    getEffect()->setSinusSpeed(settings.value("Visualizer/scrollerSinusSpeed", 0.1).toDouble());
    getEffect()->setScrollSpeed(settings.value("Visualizer/scrollerScrollSpeed", 3).toInt());
    getEffect()->setVerticalScrollPosition(settings.value("Visualizer/scrollerVerticalPosition", 0).toInt());
    getEffect()->setFontScaleX(settings.value("Visualizer/scrollerFontXScale", 1).toInt());
    getEffect()->setFontScaleY(settings.value("Visualizer/scrollerFontYScale", 2).toInt());
    getEffect()->setPrinterFontScaleX(settings.value("Visualizer/printerFontXScale", 1).toInt());
    getEffect()->setPrinterFontScaleY(settings.value("Visualizer/printerFontYScale", 1).toInt());
    getEffect()->setReflectionEnabled(settings.value("Visualizer/reflection", true).toBool());
    getEffect()->setStarsEnabled(settings.value("Visualizer/starfield", true).toBool());
    getEffect()->setCustomScrolltextEnabled(settings.value("Visualizer/scrollerCustomText", false).toBool());
    getEffect()->setCustomScrolltext(settings.value("Visualizer/scrollerCustomTextContent", "").toString());
    getEffect()->setNumberOfStars(settings.value("Visualizer/starfieldAmount", 1000).toInt());
    getEffect()->setStarsDirection(settings.value("Visualizer/starfieldDirection", "right").toString());
    getEffect()->setStarSpeed(settings.value("Visualizer/starfieldSpeed", 5).toInt());
    getEffect()->setScrollerEnabled(settings.value("Visualizer/scroller", true).toBool());
    getEffect()->setPrinterEnabled(settings.value("Visualizer/printer", true).toBool());
    getEffect()->setVuMeterEnabled(settings.value("Visualizer/vuMeter", true).toBool());
    getEffect()->setScrollerReflectionColor(
        QColor(settings.value("Visualizer/reflectionColor", "#00032e").toString()));
    getEffect()->setSinusFontScalingEnabled(settings.value("Visualizer/scrollerSinusFontScaling", false).toBool());
    getEffect()->setRotatingObjectEnabled(settings.value("Visualizer/rotatingObject", false).toBool());
    getEffect()->setRotatingObjectOrbit(settings.value("Visualizer/rotatingObjectOrbit", true).toBool());
    getEffect()->setRotatingObjectColor(settings.value("Visualizer/rotatingObjectMaterialColor", "#009379").toString());
    getEffect()->setRotatingObjectSize(settings.value("Visualizer/rotatingObjectModelSize", 21).toInt());
    getEffect()->setRotatingObjectFocalLength(settings.value("Visualizer/rotatingObjectFocalLength", 220).toInt());
    getEffect()->setRotatingObjectOrbitSize(settings.value("Visualizer/rotatingObjectOrbitSize", 245).toInt());
    getEffect()->setRotatingObjectOrbitSpeed(settings.value("Visualizer/rotatingObjectOrbitSpeed", 2).toInt());
    getEffect()->setRotatingObjectColorWireframe(
        settings.value("Visualizer/rotatingObjectWireframeColor", "#ffffff").toString());
    getEffect()->
            setRotatingObjectWireframeEnabled(settings.value("Visualizer/rotatingObjectWireframe", false).toBool());
    getEffect()->setRotatingObjectModel(settings.value("Visualizer/rotatingObjectModel", "cube").toString());
    getEffect()->setRotatingObjectMaterial(settings.value("Visualizer/rotatingObjectMaterial", "blinn").toString());
    getEffect()->setRasterBarsEnabled(settings.value("Visualizer/rasterBars", false).toBool());
    getEffect()->setNumberOfRasterBars(settings.value("Visualizer/rasterBarsAmount", 8).toInt());
    getEffect()->setRasterBarsSpeed(settings.value("Visualizer/rasterBarsSpeed", 35).toInt());
    getEffect()->setRasterBarsBarHeight(settings.value("Visualizer/rasterBarsHeight", 16).toInt());
    getEffect()->setRasterBarsVerticalSpacing(settings.value("Visualizer/rasterBarsVerticalSpacing", 16).toInt());
    getEffect()->setRasterBarsOpacity(settings.value("Visualizer/rasterBarsOpacity", 100).toInt());
    getEffect()->setKeepAspectRatio(settings.value("Visualizer/maintainAspectRatio", false).toBool());
    getEffect()->setResolutionWidth(settings.value("Visualizer/resolutionWidth", 320).toInt());
    getEffect()->setResolutionHeight(settings.value("Visualizer/resolutionHeight", 256).toInt());

    getEffect()->setScrollerFont(settings.value("Visualizer/scrollerFontImage",
                                                dataPath + "/resources/visualizer/bitmapfonts/angels_font.png").
        toString());
    getEffect()->setPrinterFont(settings.value("Visualizer/printerFontImage",
                                               dataPath + "/resources/visualizer/bitmapfonts/angels_font.png").
        toString());

    getEffect()->setReflectionOpacity(settings.value("Visualizer/reflectionOpacity", 50).toInt());

    playlistRowHeight = settings.value("Appearance/playlistItemRowHeight", "30").toInt();
    playlistsRowHeight = settings.value("Appearance/playlistRowHeight", "30").toInt();
    nowPlayingFontSize = settings.value("Appearance/nowPlayingFontSize", "16").toInt();

    setColorVisualizerTop(colorVisualizerTop);
    setColorVisualizerBottom(colorVisualizerBottom);
    setColorVisualizerMiddle(colorVisualizerMiddle);
    setColorVisualizerBackground(colorVisualizerBackground);

    setColorVisualizerPeakColor(colorVisualizerPeak);
    setVuMeterPeaksEnabled(vuMeterPeaksEnabled);
    setVuMeterPeaksHeight(vuMeterPeaksHeight);

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
            [this](const int logicalIndex, const int newSize) {
                if (ui->tableInfo->horizontalHeaderItem(logicalIndex) != nullptr) {
                    if (const QString columnText = ui->tableInfo->horizontalHeaderItem(logicalIndex)->text();
                        columnText == "Name") {
                        infoNameWidth = newSize;
                    } else if (columnText == "Value") {
                        infoValueWidth = newSize;
                    }
                }
            });

    addDebugText("Settings, reverbpreset: " + reverbPreset);
    addDebugText("Settings, reverbEnabled: " + QString::number(reverbEnabled));

    addDebugText("isVolumeMuted: " + QString::number(isVolumeMuted));
    addDebugText("resetVolume: " + QString::number(resetVolume));

    ui->volumeSlider->setDefaultValue(100);

    int vol = settings.value("Internal/volume", 100).toInt();

    if (resetVolume) {
        ui->volumeSlider->setValue(resetVolumeValue);
    } else {
        ui->volumeSlider->setValue(vol);
    }

    isVolumeMuted = settings.value("Internal/mute", false).toBool();

    if (resetVolume) {
        isVolumeMuted = false;
    }

    addDebugText("isVolumeMuted: " + QString::number(isVolumeMuted));

    if (isVolumeMuted) {
        ui->checkBoxVolumeOn->setCheckState(Qt::Unchecked);
    } else {
        ui->checkBoxVolumeOn->setCheckState(Qt::Checked);
    }

    auto &sm = SoundManager::getInstance();
    sm.Init(FMOD_OUTPUTTYPE_NOSOUND, ""); // set sound device to silent

    sm.setNormalizeEnabled(normalizeEnabled);
    sm.setNormalizeFadeTime(normalizeFadeTime);
    sm.setNormalizeMaxAmp(normalizeMaxAmp);
    sm.setNormalizeThreshold(normalizeThreshold);
    sm.setReverbPreset(reverbPreset);
    sm.setReverbEnabled(reverbEnabled);

    fileInfoParser = new FileInfoParser();

    refreshInfoTimer = 0;

    Timer = new QTimer(this);
    connect(Timer, SIGNAL(timeout()), this, SLOT(timerProgress()));

    Timer->start(16);

    qDebug() << "timerProgress started";
    playStarted = false;
    loaded = false;
    buttonNextClicked = true;

    ui->positionSlider->setMaximum(0);

    if (!QDir(userPath).exists()) {
        QDir().mkdir(userPath);
    }

    if (!QDir(userPath + PLAYLISTS_DIR).exists()) {
        QDir().mkdir(userPath + PLAYLISTS_DIR);
    }

    if (!QDir(userPath + PLUGINS_DIR).exists()) {
        QDir().mkdir(userPath + PLUGINS_DIR);
    }

    if (!QDir(userPath + PLUGIN_libsidplayfp_DIR).exists()) {
        QDir().mkdir(userPath + PLUGIN_libsidplayfp_DIR);
    }

    if (!QDir(userPath + PLUGINS_CONFIG_DIR).exists()) {
        QDir().mkdir(userPath + PLUGINS_CONFIG_DIR);
    }

    QDir directory(userPath + PLAYLISTS_DIR);

    QStringList playlists;

    if (!QFileInfo::exists(userPath + PLAYLISTS_DIR + QDir::separator() + PLAYLIST_DEFAULT_FILENAME)) {
        playlists.append(PLAYLIST_DEFAULT_FILENAME);
    }

    // remove PLAYLIST_DEFAULT_FILENAME and the put it first in playlists
    playlists.append(directory.entryList(QStringList() << "*.m3u" << "*.M3U" << "*.m3u8" << "*.M3U8", QDir::Files));
    playlists.removeOne(PLAYLIST_DEFAULT_FILENAME);
    playlists.insert(0, PLAYLIST_DEFAULT_FILENAME);

    addDebugText("Loading " + QString::number(playlists.count()) + " playlists from " + userPath + PLAYLISTS_DIR);

    changeStyleSheetColor();

    // read all columns orders, size etc. from settings for all playlists
    settings.beginGroup("InternalPlaylists");

    QStringList playlistsGeometryKeys = settings.childKeys();
    QMap<QString, QByteArray> playlistsGeometryMap;

    for (const QString &plsGKey: playlistsGeometryKeys) {
        QByteArray data = settings.value(plsGKey).toByteArray();
        playlistsGeometryMap.insert(plsGKey, data);
    }

    settings.endGroup();

    QStringList previousSortedPlaylists = settings.value("Internal/playlistsOrder").toStringList();
    QStringList sortedPlaylists = sortPreservingOrder(playlists, previousSortedPlaylists);

    foreach(QString filename, sortedPlaylists) {
        QFileInfo f(filename);

        auto tv = new DraggableTableView();
        tv->setDragBackgroundColor(QColor(colorMain.left(7)));
        tv->setDragTextColor(QColor(colorMainText.left(7)));
        tv->setupDelegate(); // has to be called after colors are set

        auto pm = new PlaylistModel(this);
        auto proxyModel = new QSortFilterProxyModel(pm); // create proxy
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
        columns << tr("TITLE") << tr("FORMAT") << tr("LENGTH") << tr("SUBSONG");

        // there was no existing default playlist
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

        auto newItem = new QListWidgetItem;
        newItem->setText(f.fileName());

        if (f.fileName() == PLAYLIST_DEFAULT_FILENAME) {
            QFont fontItalic = newItem->font();
            fontItalic.setItalic(true);
            newItem->setFont(fontItalic);
        }

        newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
        newItem->setFlags(
            newItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled |
            Qt::ItemIsDropEnabled);
        ui->listWidget->setDragEnabled(true);
        ui->listWidget->setAcceptDrops(true);
        ui->listWidget->setDropIndicatorShown(true);
        ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
        ui->listWidget->setDefaultDropAction(Qt::MoveAction);
        ui->listWidget->insertItem(ui->listWidget->count(), newItem);
        ui->listWidget->setDragBackgroundColor(QColor(colorMain.left(7)));
        ui->listWidget->setDragTextColor(QColor(colorMainText.left(7)));
        auto item = new MyItemDelegate(this);
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

    addDebugText("current playlist: " + currentPlaylist);
    QList<QListWidgetItem *> items = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);

    int row = 0;

    for (int i = 0; i < items.count(); i++) {
        row = items.at(i)->listWidget()->row(items.at(i));
        break;
    }

    setPlaylistRowHeight(playlistRowHeight);
    ui->listWidget->setCurrentRow(row);

    on_listWidget_itemClicked(ui->listWidget->currentItem());

    // first save default layout;
    this->setHidden(true);

    QRect BZRrect = geometry();
    BZRrect.setTop(200);
    BZRrect.setLeft(200);
    setGeometry(BZRrect);
    defaultGeometry = saveGeometry();
    this->setHidden(false);
    defaultState = saveState();
    dockingState = dockManager->saveState();

    if (settings.value("shuffle", false).toBool()) {
        resetShuffle(currentPlaylist);
    }

    QUrl imageUrl(PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_URL);
    filedownloader = new FileDownloader(imageUrl, this);

    qint64 currentSeconds = QDateTime::currentSecsSinceEpoch();

    if (PLUGIN_libsidplayfp_LIB != "") {
        if (bundledHvscSonglengthsUpdateFrequency == "Never") {
            // do nothing
        } else if (bundledHvscSonglengthsUpdateFrequency == "At every start") {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (bundledHvscSonglengthsUpdateFrequency == "Daily" && currentSeconds -
                   bundledHvscSonglengthsDownloadEpoch >= 86400) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (bundledHvscSonglengthsUpdateFrequency == "Weekly" && currentSeconds -
                   bundledHvscSonglengthsDownloadEpoch >=
                   604800) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        } else if (bundledHvscSonglengthsUpdateFrequency == "Monthly" && currentSeconds -
                   bundledHvscSonglengthsDownloadEpoch >=
                   2629743) {
            connect(filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
        }
    }

    connect(ui->samples->horizontalHeader(), &QHeaderView::sectionResized,
            [this](const int logicalIndex, const int newSize) {
                if (ui->samples->horizontalHeaderItem(logicalIndex) != nullptr) {
                    if (const QString columnText = ui->samples->horizontalHeaderItem(logicalIndex)->text();
                        columnText == "#") {
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
            [this](const int logicalIndex, const int newSize) {
                if (ui->instruments->horizontalHeaderItem(logicalIndex) != nullptr) {
                    if (const QString columnText = ui->instruments->horizontalHeaderItem(logicalIndex)->text();
                        columnText == "#") {
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
    dockManager->restoreState(settings.value("Internal/dockingState").toByteArray());
    restoreGeometry(settings.value("Internal/geometry").toByteArray());
    restoreState(settings.value("Internal/windowState").toByteArray());
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

    if (QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        !l.isEmpty()) {
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
    ui->checkBoxLoopPoints->installEventFilter(this);
    ui->pushButtonNewPlaylist->installEventFilter(this);

    ui->Debug->moveCursor(QTextCursor::StartOfLine);

    on_buttonStop_clicked();
    updateButtons();
    highlightPlaylistItem(currentPlaylist, currentRow);
    checkCommandLine(argc, argv);
}

void MainWindow::checkCommandLine(int argc, char *argv[]) {
    QStringList args = qApp->arguments();

    if (args.count() <= 1) return; // we've got no songs/dirs from command line

    args.removeFirst();
    QList<QUrl> urls;
    for (const auto &item: args) {
        urls.append(QUrl().fromLocalFile(item));
    }

    const int rowCountBeforeAddSong = tableWidgetPlaylists[currentPlaylist]->model()->rowCount();

    addSong(urls, 0, PLAYLIST_DEFAULT_FILENAME, false);

    if ((allowOnlyOneInstance && !enqueueItems) || !allowOnlyOneInstance) {
        on_listWidget_itemClicked(ui->listWidget->item(0));
        const QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
        removeHighlight();
        currentRow = rowCountBeforeAddSong;
        playSongAtRow(currentRow);
    }
}

bool MainWindow::initializeSocket() {
    bool instanceExists = false;

    tcpServer = new QTcpServer(this);
    // if we can't listen with server, there is already an instance of the application running
    // so we don't create a server, we create a client instead
    // the client will send command line argumens (filepaths) to the server, when the server has got it,
    // the client will disconnect and close
    if (!tcpServer->listen(QHostAddress::LocalHost, 9860)) {
        instanceExists = true;
    }

    if (!instanceExists) {
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    } else {
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

void MainWindow::createMenuWindowTabs() {
    ui->menuWindow->addSeparator();

    foreach(ads::CDockWidget* widg, dockWidgets) {
        const auto action = new QAction(widg->windowTitle());

        if (action->text().toLower().endsWith(".m3u") || action->text().toLower().endsWith(".m3u8")) {
            action->setText("Playlist");
        }

        action->setCheckable(true);
        action->setChecked(!widg->isClosed());
        connect(action, &QAction::triggered, this, [&, this, widg] { slot_dockWidgetMenuChecked(widg); });
        connect(widg, SIGNAL(visibilityChanged(bool)), SLOT(dockWindowClosed(bool)));
        ui->menuWindow->addAction(action);
    }
}

void MainWindow::dockWindowClosed(bool b) {
    foreach(ads::CDockWidget* dock, dockWidgets) {
        foreach(QAction *menuAction, ui->menuWindow->actions()) {
            if (menuAction->text() == dock->windowTitle() || (
                    (dock->windowTitle().toLower().endsWith(".m3u") || dock->windowTitle().toLower().endsWith(".m3u8"))
                    && menuAction->text() == "Playlist")) {
                menuAction->setChecked(!dock->isClosed());
            }
        }
    }
}

void MainWindow::slot_dockWidgetMenuChecked(ads::CDockWidget *d) const {
    addDebugText("Clicked " + d->windowTitle());

    if (d->isClosed()) {
        d->toggleView(true);
    } else {
        d->closeDockWidget();
    }
}

void MainWindow::on_positionSlider_sliderReleased() const {
    setPosition();
}

void MainWindow::setPosition(const int offset) const {
    const int currentPos = ui->positionSlider->value();
    int targetPos = currentPos + offset;

    if (loopPointsState == B_SET && targetPos > loopPointB) {
        targetPos = static_cast<int>(loopPointA);
    } else {
        if (const int maxPos = ui->positionSlider->maximum(); targetPos >= maxPos) {
            targetPos--;
        } else if (targetPos < 0) {
            targetPos = 0;
        }
    }

    SoundManager::getInstance().setPosition(targetPos, FMOD_TIMEUNIT_MS);
    addDebugText("Set position to " + QString::number(targetPos));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (isMenuBarHiddenChecked && handleMenuBarHiddenEvents(event)) {
        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent const *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_F5) {
            ui->buttonPlay_2->click();
        } else if (keyEvent->key() == Qt::Key_Right) {
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
                setPosition(3000);
            } else if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
                setPosition(60000);
            } else if (QApplication::keyboardModifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
                setPosition(300000);
            } else {
                setPosition(10000);
            }
        } else if (keyEvent->key() == Qt::Key_Left) {
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
                setPosition(-3000);
            } else if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
                setPosition(-60000);
            } else if (QApplication::keyboardModifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
                setPosition(-300000);
            } else {
                setPosition(-10000);
            }
        }

        if (obj->parent() != nullptr && obj->parent()->objectName() == "Playlist") {
            keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Delete) {
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
        } else if (obj == ui->listWidget) {
            keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Delete) {
                addDebugText("Deleting playlist");
                deletePlaylist();
            }
        } else if (obj == visualizerFullScreen) {
            keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Escape ||
                keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                visualizerFullScreen->hide();
            }
        } else if (obj == trackerFullScreen) {
            keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Escape || keyEvent->key() ==
                Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                trackerFullScreen->hide();
            }
        }
    } else if (event->type() == QEvent::Enter) {
        if (obj == ui->buttonPrev) {
            ui->buttonPrev->setIcon(icons["prevHover"]);
        } else if (obj == ui->buttonNext) {
            ui->buttonNext->setIcon(icons["nextHover"]);
        } else if (obj == ui->buttonStop) {
            ui->buttonStop->setIcon(icons["stopHover"]);
        } else if (obj == ui->pushButtonNewPlaylist) {
            ui->pushButtonNewPlaylist->setIcon(icons["addHover"]);
        } else if (obj == ui->buttonPlay_2) {
            if (const auto &sm = SoundManager::getInstance();
                sm.isPlaying() && !sm.isPaused()) {
                if (!sm.isWavWriterDeviceSelected()) {
                    ui->buttonPlay_2->setIcon(icons["pauseHover"]);
                }
            } else {
                ui->buttonPlay_2->setIcon(icons["playHover"]);
            }
        } else if (obj == ui->checkBoxShuffle) {
            ui->checkBoxShuffle->setIcon(icons[isShuffleEnabled() ? "shuffle-onHover" : "shuffle-offHover"]);
        } else if (obj == ui->checkBoxVolumeOn) {
            ui->checkBoxVolumeOn->setIcon(icons[isVolumeMuted ? "speaker-offHover" : "speaker-onHover"]);
        } else if (obj == ui->checkBoxLoop) {
            if (playMode == normal) {
                ui->checkBoxLoop->setIcon(icons["repeatHover"]);
            } else if (playMode == repeatPlaylist) {
                ui->checkBoxLoop->setIcon(icons["repeat-onHover"]);
            } else {
                ui->checkBoxLoop->setIcon(icons["repeat-1Hover"]);
            }
        } else if (obj == ui->checkBoxLoopPoints) {
            switch (loopPointsState) {
                case A_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-point-a-on-hover"]);
                    break;
                case B_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-on-hover"]);
                    break;
                case UNSET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-off-hover"]);
                    break;
                default: ;
            }
        }
    } else if (event->type() == QEvent::Leave) {
        if (obj == ui->buttonPrev) {
            ui->buttonPrev->setIcon(icons["prev"]);
        } else if (obj == ui->buttonNext) {
            ui->buttonNext->setIcon(icons["next"]);
        } else if (obj == ui->buttonStop) {
            ui->buttonStop->setIcon(icons["stop"]);
        } else if (obj == ui->pushButtonNewPlaylist) {
            ui->pushButtonNewPlaylist->setIcon(icons["add"]);
        } else if (obj == ui->buttonPlay_2) {
            if (const auto &sm = SoundManager::getInstance();
                sm.isPlaying() && !sm.isPaused()) {
                if (!sm.isWavWriterDeviceSelected()) {
                    ui->buttonPlay_2->setIcon(icons["pause"]);
                }
            } else {
                ui->buttonPlay_2->setIcon(icons["play"]);
            }
        } else if (obj == ui->checkBoxShuffle) {
            ui->checkBoxShuffle->setIcon(icons[isShuffleEnabled() ? "shuffle-on" : "shuffle-off"]);
        } else if (obj == ui->checkBoxVolumeOn) {
            ui->checkBoxVolumeOn->setIcon(icons[isVolumeMuted ? "speaker-off" : "speaker-on"]);
        } else if (obj == ui->checkBoxLoop) {
            if (playMode == normal) {
                ui->checkBoxLoop->setIcon(icons["repeat"]);
            } else if (playMode == repeatPlaylist) {
                ui->checkBoxLoop->setIcon(icons["repeat-on"]);
            } else {
                ui->checkBoxLoop->setIcon(icons["repeat-1"]);
            }
        } else if (obj == ui->checkBoxLoopPoints) {
            switch (loopPointsState) {
                case UNSET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-off"]);
                    break;
                case A_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-point-a-on"]);
                    break;
                case B_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-on"]);
                    break;
                default: ;
            }
        }
    } else if (event->type() == QEvent::Wheel && obj == ui->labelFilename &&
               QApplication::keyboardModifiers() == Qt::ControlModifier) {
        const auto wheelEvent = static_cast<QWheelEvent *>(event);
        const QPoint numPixels = wheelEvent->pixelDelta();
        const QPoint numDegrees = wheelEvent->angleDelta() / 8;
        const QFont font = ui->labelFilename->font();

        if (!numPixels.isNull() && numPixels.y() != 0) {
            if (font.pixelSize() + numPixels.y() <= 100 && font.pixelSize() + numPixels.y() >= 1) {
                setNowPlayingFontSize(font.pixelSize() + numPixels.y());
            }
        } else if (!numDegrees.isNull() && numDegrees.y() != 0) {
            if (const QPoint numSteps = numDegrees / 15;
                font.pixelSize() + numSteps.y() * 2 <= 100 && font.pixelSize() + numSteps.y() * 2 >= 1) {
                setNowPlayingFontSize(font.pixelSize() + numSteps.y() * 2);
            }
        }
    } else if (event->type() == QEvent::Wheel && obj == ui->visualizer &&
               QApplication::keyboardModifiers() == Qt::ControlModifier) {
        const auto wheelEvent = static_cast<QWheelEvent *>(event);
        const QPoint numPixels = wheelEvent->pixelDelta();
        const QPoint numDegrees = wheelEvent->angleDelta() / 8;
        float scale = 1;

        if (!numPixels.isNull() && numPixels.y() != 0) {
            scale = numPixels.y();
        } else if (!numDegrees.isNull() && numDegrees.y() != 0) {
            const QPoint numSteps = numDegrees / 15;
            scale = numSteps.y() * 2;
        }

        scale = scale / 40;
        scale = 1 - scale;

        if (scale * getEffect()->getResolutionWidth() >= 80 &&
            scale * getEffect()->getResolutionHeight() >= 80 &&
            scale * getEffect()->getResolutionWidth() <= 4096 &&
            scale * getEffect()->getResolutionHeight() <= 4096) {
            getEffect()->setResolutionWidth(scale * getEffect()->getResolutionWidth());
            getEffect()->setResolutionHeight(scale * getEffect()->getResolutionHeight());
        }
    } else if (obj == visualizerFullScreen) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (auto const *mouseEvent = static_cast<QMouseEvent *>(event); mouseEvent->button() == Qt::LeftButton) {
                visualizerFullScreen->hide();
            }
        }
    } else if (obj == ui->visualizer) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (QMouseEvent const *mouseEvent = static_cast<QMouseEvent *>(event);
                mouseEvent->button() == Qt::LeftButton) {
                visualizerFullScreen->showFullScreen();
            }
        }
    } else if (obj == trackerFullScreen) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                mouseEvent->button() == Qt::LeftButton) {
                trackerFullScreen->hide();
            }
        }
    } else if (obj == ui->trackerView) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                mouseEvent->button() == Qt::LeftButton) {
                trackerFullScreen->showFullScreen();
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

bool MainWindow::handleMenuBarHiddenEvents(QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate) {
        menuBar()->setHidden(true);
        altKeyPressCaught = false;
        return true;
    }

    if (const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event); keyEvent->key() == Qt::Key_Alt) {
        if (altKeyPressCaught) {
            if (event->type() == QEvent::KeyRelease && QGuiApplication::keyboardModifiers() == Qt::AltModifier) {
                menuBar()->setHidden(!menuBar()->isHidden());
                altKeyPressCaught = false;
                return true;
            }
        } else if (event->type() == QEvent::KeyPress && QApplication::keyboardModifiers() == Qt::NoModifier) {
            altKeyPressCaught = true;
            return true;
        }
    }

    return false;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
}

void MainWindow::addDebugText(const QString &debugText) const {
    ui->Debug->appendPlainText(debugText);
}

void MainWindow::refreshInfo() {
    PlaylistItem pi;
    pi.fullPath = currentPlayingFilepath;
    pi.info = SoundManager::getInstance().info;
    FileInfoParser::updateFileInfo(ui->tableInfo, &pi);

    QString title;

    if (!pi.info->title.empty()) {
        title = fromUtf8OrLatin1(pi.info->title);
    } else {
        title = fromUtf8OrLatin1(!pi.info->containerFilenames.empty()
                                     ? pi.info->containerLastFilename
                                     : pi.info->filename);
    }

    QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, title, Qt::EditRole);

    index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 8, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, fromUtf8OrLatin1(pi.info->artist), Qt::EditRole);
}

void MainWindow::timerProgress() {
    unsigned int currentMs = 0;
    const auto &sm = SoundManager::getInstance();

    if (sm.isPlaying()) {
        refreshInfoTimer++;

        if (refreshInfoTimer >= 120) {
            if (const QFileInfo fileinfo(currentPlayingFilepath);
                fileinfo.size() == 0) {
                refreshInfoTimer = 0;
                refreshInfo();
            }
        }

        if (visualizerFullScreen->isVisible()) {
            visualizerFullScreen->update();
        } else if (trackerFullScreen->isVisible() && !sm.isPaused()) {
            trackerFullScreen->update();
        } else {
            ui->visualizer->update();

            if (!sm.isPaused()) {
                ui->trackerView->update();
            }
        }

        currentMs = sm.getPosition(FMOD_TIMEUNIT_MS_REAL);

        if (currentMs == 0) {
            currentMs = sm.getPosition(FMOD_TIMEUNIT_MS);
        }

        if (!ui->positionSlider->isSliderDown()) {
            ui->labelTimer_2->setText(msToNiceStringExact(currentMs, displayMilliseconds));
            ui->positionSlider->setValue(static_cast<int>(currentMs));
        }
    }

    if (playStarted) {
        if (loopPointsState == B_SET && currentMs > loopPointB) {
            setPosition(loopPointA - loopPointB);
            return;
        }

        if (const auto &info = sm.info;
            info != nullptr && (info->isContinuousPlaybackActive || info->isSeamlessLoopActive)) {
            return;
        }

        if (currentMs >= songLengthMs || (!sm.isPlaying() && !sm.isPaused())) {
            playNextSong(false);
        }
    }
}

void MainWindow::updateButtons() {
    const auto &sm = SoundManager::getInstance();

    if (sm.isPlaying() && !sm.isPaused()) {
        if (sm.isWavWriterDeviceSelected()) {
            ui->buttonPlay_2->setEnabled(false);
            ui->buttonPlay_2->setIcon(icons["pause-disabled"]);
            ui->buttonPlay_2->setToolTip("Pause not available");
        } else {
            ui->buttonPlay_2->setIcon(icons["pause"]);
            ui->buttonPlay_2->setToolTip("Pause");
        }
    } else {
        ui->buttonPlay_2->setEnabled(true);
        ui->buttonPlay_2->setIcon(icons["play"]);
        ui->buttonPlay_2->setToolTip("Play");
    }

    ui->checkBoxShuffle->setIcon(icons[isShuffleEnabled() ? "shuffle-on" : "shuffle-off"]);

    if (playMode == repeatSong) {
        ui->checkBoxLoop->setIcon(icons["repeat-1"]);
    } else if (playMode == repeatPlaylist) {
        ui->checkBoxLoop->setIcon(icons["repeat-on"]);
    } else {
        ui->checkBoxLoop->setIcon(icons["repeat"]);
    }

    if (sm.isPlaying()) {
        if (sm.info->getSeekable() && songLengthMs != -1 && !sm.info->isContinuousPlaybackActive) {
            switch (loopPointsState) {
                case A_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-point-a-on"]);
                    break;
                case B_SET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-on"]);
                    break;
                case INACTIVE:
                    loopPointsState = UNSET;
                case UNSET:
                    ui->checkBoxLoopPoints->setIcon(icons["loop-points-off"]);
                    ui->checkBoxLoopPoints->setToolTip("Set starting loop point");
                    break;
                default: ;
            }
        } else {
            loopPointsState = DISABLED;
            ui->checkBoxLoopPoints->setIcon(icons["loop-points-off-disabled"]);
            ui->checkBoxLoopPoints->setToolTip("Loop points not available");
        }
    } else {
        loopPointsState = INACTIVE;
        ui->checkBoxLoopPoints->setIcon(icons["loop-points-off"]);
        ui->checkBoxLoopPoints->setToolTip("");
    }

    ui->checkBoxVolumeOn->setIcon(icons[isVolumeMuted ? "speaker-off" : "speaker-on"]);
}

MainWindow::~MainWindow() {
    saveSettings();
    addDebugText("Quitting...");
    delete ui;
}

void MainWindow::playNextSong(const bool forceNext) {
    addDebugText("Song ended. Time to play next song.");
    addDebugText("PlayMode: " + QString::number(playMode));

    if (!tableWidgetPlaylists.contains(currentPlaylist)) {
        // this happens if a song is playing and that playlist is deleted while playing
        on_buttonStop_clicked();
        currentRow = 0;
        currentPlaylist = PLAYLIST_DEFAULT_FILENAME;
        ui->listWidget->setCurrentRow(0);
        on_listWidget_itemClicked(ui->listWidget->currentItem());
    } else if (playMode == repeatSong && !forceNext) {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0 &&
            currentRow != tableWidgetPlaylists[currentPlaylist]->model()->rowCount()) {
            playSongAtRow(currentRow);
        } else {
            on_buttonStop_clicked();
        }
    } else if (playMode == normal || playMode == repeatPlaylist || forceNext) {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() == 0) {
            addDebugText("Playlist is empty.");
            on_buttonStop_clicked();
        } else {
            if (isShuffleEnabled()) {
                currentShufflePosition[currentPlaylist]++;

                if (!shuffleToBePlayed[currentPlaylist].isEmpty()) {
                    if (currentShufflePosition[currentPlaylist] >= shufflePlayed[currentPlaylist].size()) {
                        const unsigned int next = rand() % shuffleToBePlayed[currentPlaylist].size();
                        addDebugText("Next position shuffled: " + QString::number(next));
                        removeHighlight();
                        currentRow = shuffleToBePlayed[currentPlaylist].at(next);
                        shufflePlayed[currentPlaylist].push_back(currentRow);
                        shuffleToBePlayed[currentPlaylist].remove(next);
                    } else {
                        addDebugText("Previously had this position shuffled.");
                        removeHighlight();
                        currentRow = shufflePlayed[currentPlaylist].at(currentShufflePosition[currentPlaylist]);
                    }

                    playSongAtRow(currentRow);
                } else {
                    addDebugText("No more songs in playlist.");
                    on_buttonStop_clicked();
                }
            } else // normal or repeat playlist with no shuffle
            {
                if (currentRow < tableWidgetPlaylists[currentPlaylist]->model()->rowCount() - 1) {
                    removeHighlight();
                    if (isUpdateCurrentRowToNextEnabled) {
                        currentRow++;
                    }
                    playSongAtRow(currentRow);
                } else {
                    if (playMode == normal || playMode == repeatSong) {
                        if (isUpdateCurrentRowToNextEnabled) {
                            addDebugText("No more songs in playlist.");
                            on_buttonStop_clicked();
                        } else {
                            playSongAtRow(currentRow);
                        }
                    } else {
                        addDebugText("Repeat playlist.");
                        removeHighlight();
                        if (isUpdateCurrentRowToNextEnabled) {
                            currentRow = 0;
                        }
                        playSongAtRow(currentRow);
                    }
                }
            }
        }
    }

    isUpdateCurrentRowToNextEnabled = true;
}

void MainWindow::highlightPlaylistItem(const QString &playlist, const int row) {
    const QModelIndex index = tableWidgetPlaylists[playlist]->model()->index(row, 0, QModelIndex());
    tableWidgetPlaylists[playlist]->model()->setData(index, row, Qt::ForegroundRole);
    tableWidgetPlaylists[currentPlaylist]->update();
    tableWidgetPlaylists[currentPlaylist]->repaint();
}

void MainWindow::resetShuffle(const QString &playlist) {
    addDebugText("Reset shuffle for playlist '" + playlist + "'");
    shuffleToBePlayed[playlist].clear();
    shufflePlayed[playlist].clear();

    currentShufflePosition[playlist] = 0;

    for (unsigned int e = 0; e < tableWidgetPlaylists[playlist]->model()->rowCount(); e++) {
        shuffleToBePlayed[playlist].push_back(e);
    }

    if (tableWidgetPlaylists[playlist]->model()->rowCount() > 0 &&
        currentRow < tableWidgetPlaylists[playlist]->model()->rowCount() - 1) {
        shufflePlayed[playlist].push_back(currentRow);
        shuffleToBePlayed[playlist].remove(currentRow);
    }
}

QString MainWindow::getCurrentPlaylist() const {
    return currentPlaylist;
}

QString MainWindow::getSelectedPlaylist() const {
    return selectedPlaylist;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    quit();
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        this->setWindowTitle(isMinimized() ? windowTitle : PROJECT_NAME_VERSIONED);

        if (isMinimized() == true && isSystrayOnMinimizeEnabled && isSystrayOnMinimizeChecked) {
            wasMaximized = false;
            hide();
        }

        if (isMaximized() == true && isSystrayOnMinimizeEnabled && isSystrayOnMinimizeChecked) {
            wasMaximized = true;
        }
    }

    return QMainWindow::changeEvent(event);
}

bool MainWindow::loadSound(const QString &fullPath, const int subsong) {
    auto &sm = SoundManager::getInstance();

    addDebugText("Try to load sound (playlistitem): " + fullPath);
    ui->buttonPlay_2->setIcon(icons["pause"]);
    const QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 0);
    tableWidgetPlaylists[currentPlaylist]->scrollTo(index);
    repaint();

    auto *info = new Info();
    info->clearMemory();
    info->clear();
    info->isPlayModeRepeatSongEnabled = playMode == repeatSong;
    info->isFmodSeamlessLoopEnabled = getFmodSeamlessLoopEnabled();
    info->currentSubsong = subsong;

    const bool loadOK = sm.loadSound(fullPath, info);

    if (loadOK) {
        addDebugText("Loaded sound succesfully.");

        loaded = true;
        currentPlayingFilepath = fullPath;
    } else {
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        //addDebugText("x: " + QString::number(dockWidgetPlaylistRect.left()) + " y: " + dockWidgetPlaylistRect.top() + "w:" + dockWidgetPlaylistRect.width() + "h:" + dockWidgetPlaylistRect.height());
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    if (!event->mimeData()->hasFormat("text/uri-list")) {
        event->ignore();
        dropWidget = DROP_IGNORE;
        return;
    }

    constexpr int UNKNOWN_OFFSET = 64;

    if (dockWidgetPlaylists->isCurrentTab()) {
        QRect dockWidgetPlaylistsRect = dockWidgetPlaylists->geometry();
        const QPoint pointPlaylists = dockWidgetPlaylists->mapTo(this, dockWidgetPlaylists->pos());

        dockWidgetPlaylistsRect.setRight(pointPlaylists.x() + dockWidgetPlaylistsRect.width());
        dockWidgetPlaylistsRect.setBottom(
            pointPlaylists.y() + dockWidgetPlaylistsRect.height() - UNKNOWN_OFFSET + 16);
        dockWidgetPlaylistsRect.setLeft(pointPlaylists.x());
        dockWidgetPlaylistsRect.setTop(pointPlaylists.y() - UNKNOWN_OFFSET);

        if (dockWidgetPlaylistsRect.contains(event->position().toPoint())) {
            event->acceptProposedAction();
            dropWidget = DROP_TO_PLAYLISTS;
        } else if (dockWidgetPlaylist->isCurrentTab()) {
            QRect dockWidgetPlaylistRect = dockWidgetPlaylist->geometry();
            const QPoint pointPlaylist = dockWidgetPlaylist->mapTo(this, dockWidgetPlaylist->pos());

            dockWidgetPlaylistRect.setRight(pointPlaylist.x() + dockWidgetPlaylistRect.width());
            dockWidgetPlaylistRect.setBottom(
                pointPlaylist.y() + dockWidgetPlaylistRect.height() - UNKNOWN_OFFSET / 2 + 8);
            dockWidgetPlaylistRect.setLeft(pointPlaylist.x());
            dockWidgetPlaylistRect.setTop(pointPlaylist.y() - UNKNOWN_OFFSET);

            if (dockWidgetPlaylistRect.contains(event->position().toPoint())) {
                event->acceptProposedAction();
                dropWidget = DROP_TO_PLAYLIST;
            } else {
                event->ignore();
                dropWidget = DROP_IGNORE;
            }
        } else {
            event->ignore();
            dropWidget = DROP_IGNORE;
        }
    } else if (dockWidgetPlaylist->isCurrentTab()) {
        QRect dockWidgetPlaylistRect = dockWidgetPlaylist->geometry();
        const QPoint pointPlaylist = dockWidgetPlaylist->mapTo(this, dockWidgetPlaylist->pos());

        dockWidgetPlaylistRect.setRight(pointPlaylist.x() + dockWidgetPlaylistRect.width());
        dockWidgetPlaylistRect.setBottom(
            pointPlaylist.y() + dockWidgetPlaylistRect.height() - UNKNOWN_OFFSET / 2 + 8);
        dockWidgetPlaylistRect.setLeft(pointPlaylist.x());
        dockWidgetPlaylistRect.setTop(pointPlaylist.y() - UNKNOWN_OFFSET);

        if (dockWidgetPlaylistRect.contains(event->position().toPoint())) {
            event->acceptProposedAction();
            dropWidget = DROP_TO_PLAYLIST;
        } else if (dockWidgetPlaylists->isCurrentTab()) {
            QRect dockWidgetPlaylistsRect = dockWidgetPlaylists->geometry();
            const QPoint pointPlaylists = dockWidgetPlaylists->mapTo(this, dockWidgetPlaylists->pos());

            dockWidgetPlaylistsRect.setRight(pointPlaylists.x() + dockWidgetPlaylistsRect.width());
            dockWidgetPlaylistsRect.setBottom(
                pointPlaylists.y() + dockWidgetPlaylistsRect.height() - UNKNOWN_OFFSET + 16);
            dockWidgetPlaylistsRect.setLeft(pointPlaylists.x());
            dockWidgetPlaylistsRect.setTop(pointPlaylists.y() - UNKNOWN_OFFSET);

            if (dockWidgetPlaylistsRect.contains(event->position().toPoint())) {
                event->acceptProposedAction();
                dropWidget = DROP_TO_PLAYLISTS;
            } else {
                event->ignore();
                dropWidget = DROP_IGNORE;
            }
        } else {
            event->ignore();
            dropWidget = DROP_IGNORE;
        }
    } else {
        event->ignore();
        dropWidget = DROP_IGNORE;
    }
}

void MainWindow::showCheckBoxLoopPoints(const bool show) {
    ui->checkBoxLoopPoints->setVisible(show);
    isShownCheckBoxLoopPoints = show;
}

void MainWindow::dropEvent(QDropEvent *event) {
    if (dropWidget == DROP_TO_PLAYLIST) {
        const QList<QUrl> list = event->mimeData()->urls();
        addSong(list, 0, ui->listWidget->currentItem()->text(), false);
    } else if (dropWidget == DROP_TO_PLAYLISTS) {
        const QList<QUrl> list = event->mimeData()->urls();
        addSong(list, 0, ui->listWidget->currentItem()->text(), true);
    }
}

// gets all files in the given directory recursively as a QStringList
QStringList MainWindow::getFilesRecursive(const QString &dirName, const QString &extension, const bool clearStaticVar) {
    static QStringList list;

    if (clearStaticVar) {
        list.clear();
    }

    QDir directory(dirName);

    if (extension != "") {
        QStringList filters;
        filters << "*." + extension;
        directory.setNameFilters(filters);
    } else {
        directory.setFilter(QDir::AllEntries);
    }

    QStringList dirs = directory.entryList(QDir::Dirs | QDir::System | QDir::Hidden);
    QStringList files = directory.entryList(QDir::Files | QDir::System | QDir::Hidden);

    for (int i = 0; i < dirs.count(); i++) {
        if (dirs[i] == "." || dirs[i] == "..") continue;
        getFilesRecursive(directory.path() + '/' + dirs[i], extension, false);
    }

    for (int k = 0; k < files.count(); k++) {
        list.append(directory.path() + '/' + files[k]);
    }

    return list;
}

/*
 * Takes a Qlist of QUrl:s and converts them to a QStringList
 * This QStringList is then passed on to addSong(QStringList)
 * Returns false if an empty dir was drag'n'dropped, otherwise true
 */
bool MainWindow::addSong(const QList<QUrl> &urls, const int row, const QString &playlistName,
                         const bool createNewPlaylist) {
    QListIterator it(urls);
    QStringList stringList;

    while (it.hasNext()) {
        QUrl url = it.next();
        addDebugText("opening url: " + url.toString());

        if (url.isLocalFile()) {
            QFile file(url.toLocalFile());

            if (QFileInfo fileinfo(file); !fileinfo.isDir()) {
                stringList.append(fileinfo.filePath());
            } else {
                // it's a dir, get all files recursively
                QStringList stringListNew = getFilesRecursive(fileinfo.filePath());

                for (int i = 0; i < stringListNew.size(); i++) {
                    stringList.append(stringListNew.at(i));
                }
            }
        } else // it's a (one) url
        {
            stringList.append(url.toString());
        }
    }
    // this may happen if drag'n'drop an empty dir
    if (stringList.size() > 0) {
        addSong(stringList, row, playlistName, createNewPlaylist);
        return true;
    }

    return false;
}

/*
 * Takes a QStringList of filenames, rownumber and a playlist as input.
 * Adds all filenames to that playlist, inserting it into that row.
 * If createNewPlaylist is true, creates a new playlist with that playlist name
 * previously specified.
 */
void MainWindow::addSong(const QStringList &filenames, int row, QString playlistName, const bool createNewPlaylist) {
    QListIterator it(filenames);

    while (it.hasNext()) {
        QString filenameFullPath = it.next();
        QFile file(filenameFullPath);
        QFileInfo fileInfo(file.fileName());
        QString filename(fileInfo.fileName());

        QStringList ignorePrefixList = getIgnorePrefix().split(";");
        QStringListIterator itIgnorePrefixFiles(ignorePrefixList);
        QStringList ignoreSuffixList = getIgnoreSuffix().split(";");
        QStringListIterator itIgnoreSuffixFiles(ignoreSuffixList);
        bool ignoreThisFile = false;

        while (itIgnorePrefixFiles.hasNext()) {
            if (QString ignoreFilePrefix = itIgnorePrefixFiles.next() + ".";
                filename.startsWith(ignoreFilePrefix, Qt::CaseInsensitive)) {
                ignoreThisFile = true;
            }
        }

        if (ignoreThisFile) {
            continue;
        }

        while (itIgnoreSuffixFiles.hasNext()) {
            if (QString ignoreFileSuffix = "." + itIgnoreSuffixFiles.next();
                filename.endsWith(ignoreFileSuffix, Qt::CaseInsensitive)) {
                ignoreThisFile = true;
            }
        }

        if (ignoreThisFile) {
            continue;
        }

        addDebugText("Added " + filenameFullPath);

        if (filenameFullPath.endsWith(".m3u", Qt::CaseInsensitive) ||
            filenameFullPath.endsWith(".m3u8", Qt::CaseInsensitive)) {
            if (createNewPlaylist) {
                // create a new playlist with the filename and add all those files to that playlist
                playlistName = createPlaylist(filename);
            }

            vector<PlaylistItem *> playlistEntries = getPlayListEntriesM3u(filenameFullPath);

            for (unsigned int e = 0; e < playlistEntries.size(); e++) {
                QString length = "";

                if (playlistEntries.at(e)->length > 0) {
                    length = msToNiceStringExact(playlistEntries.at(e)->length, displayMilliseconds);
                }

                addPlaylistEntry(tableWidgetPlaylists[playlistName],
                                 tableWidgetPlaylists[playlistName]->model()->rowCount(), playlistEntries.at(e)->title,
                                 playlistEntries.at(e)->fileFormat, length, playlistEntries.at(e)->startSubsongPlayList,
                                 playlistEntries.at(e)->fullPath, playlistEntries.at(e)->length,
                                 playlistEntries.at(e)->artist);

                //playlists[playlistName].append(playlistEntries.at(e));

                //addDebugText("Added from m3u: " + QUrl::fromPercentEncoding(playlistEntries[e]->fullPath.toStdString().c_str()));
            }
        } else {
            addPlaylistEntry(tableWidgetPlaylists[playlistName],
                             tableWidgetPlaylists[playlistName]->model()->rowCount(), filename, "", nullptr, 0,
                             filenameFullPath, 0, "");
            //playlists[playlistName].append(new PlaylistItem(SoundManager::getInstance().info,filenameFullPath,0));
        }
    }
}

void MainWindow::addPlaylistEntry(const QTableView *table, const int rowPosition, const QString &filename,
                                  const QString &fileFormat,
                                  const QString &length, const int subsong, const QString &fullPath,
                                  const int lengthInt, const QString &artist) {
    table->model()->insertRows(rowPosition, 1, QModelIndex());

    QString subsongStr = QString::number(subsong);

    if (subsongStr == "0") {
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

void MainWindow::getLength() {
    if (const auto &sm = SoundManager::getInstance(); sm.isPlaying()) {
        songLengthMs = sm.getLength(FMOD_TIMEUNIT_MS);
        addDebugText("songLengthMs: " + QString::number(songLengthMs));

        if (songLengthMs == 0 || songLengthMs == -1) {
            songLengthMs = sm.getLength(FMOD_TIMEUNIT_MS_REAL);
            addDebugText("songLengthMs: " + QString::number(songLengthMs));
        }

        if (songLengthMs == 0) {
            songLengthMs = -1;
        }
        //        if(songLengthMs==-1 && playlists[currentPlaylist].at(currentRow)->length>0)
        //        {
        //            songLengthMs=playlists[currentPlaylist].at(currentRow)->length;
        //        }

        ui->positionSlider->setMaximum(songLengthMs == -1 ? 0 : songLengthMs);

        ui->labelFileLength_2->setText(msToNiceStringExact(songLengthMs, displayMilliseconds));
    } else {
        ui->positionSlider->setMaximum(0);

        ui->labelFileLength_2->setText(displayMilliseconds ? "0:00.000" : "0:00");
    }
}

void MainWindow::on_buttonPlay_2_clicked() {
    if (loaded) {
        if (const auto &sm = SoundManager::getInstance();
            sm.isPlaying() && !sm.isPaused()) {
            playAction->setText("Play");
            sm.pause(true);
        } else {
            playAction->setText("Pause");
            sm.pause(false);
        }
    } else {
        if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0) {
            if (currentRow < 0) {
                removeHighlight();
                currentRow++;
            }

            playSongAtRow(currentRow);
            playAction->setText("Pause");
        } else {
            addFiles();

            if (tableWidgetPlaylists[currentPlaylist]->model()->rowCount() > 0) {
                playSongAtRow(currentRow);
                playAction->setText("Pause");
            }
        }
    }

    updateButtons();
    isUpdateCurrentRowToNextEnabled = true;
}

void MainWindow::resetToDefaultColors() {
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

void MainWindow::on_playlist_itemDoubleClicked(const QModelIndex &index) {
    addDebugText("dbl click! " + index.model()->data(index).toString());
    addDebugText("dbl click! " + QString::number(index.row()));

    // remove highlighted playlist
    // there might by a state where no playlist is highlighted if a user
    // removed a plying playlist
    if (const QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        l.size() > 0) {
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
    }

    removeHighlight();
    currentPlaylist = ui->listWidget->currentItem()->text();

    if (isShuffleEnabled()) {
        resetShuffle(currentPlaylist);
    }

    removeHighlight();
    currentRow = index.row();

    playSongAtRow(currentRow);
    isUpdateCurrentRowToNextEnabled = true;
}

void MainWindow::updateScrollText() const {
    if (!loaded) return;

    if (ui->visualizer->getEffect()->getCustomScrolltextEnabled()) {
        // TODO this will only be blank after program start
        ui->visualizer->getEffect()->setScrollText(ui->visualizer->getEffect()->getCustomScrolltext());
    } else {
        const auto &info = SoundManager::getInstance().info;

        QString visualizerText = "";
        visualizerText = fromUtf8OrLatin1(info->artist);

        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(info->game);

        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(info->comments);

        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(info->copyright);

        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        visualizerText += fromUtf8OrLatin1(info->date);

        if (!visualizerText.isEmpty()) {
            visualizerText += " ";
        }

        if (info->instruments != nullptr) {
            for (int i = 0; i < info->numInstruments; i++) {
                visualizerText += fromUtf8OrLatin1(info->instruments[i]) + QString(" ");
            }
        }

        if (info->samples != nullptr) {
            for (int i = 0; i < info->numSamples; i++) {
                visualizerText += fromUtf8OrLatin1(info->samples[i]) + QString(" ");
            }
        }

        ui->visualizer->getEffect()->setScrollText(visualizerText);
    }
}

void MainWindow::playSongAtRow(int rowProvided) {
    QString fullPath = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 4).data().toString();
    addDebugText("Now playing and loading sound " + fullPath);

    QString subsong = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 3).data().toString();

    if (subsong != "") {
        subsong = "[" + subsong + "]";
    }

    // addDebugText("startSubsongPlayList: " + QString::number(playlists[currentPlaylist].at(playlistNumber)->startSubsongPlayList));
    currentSubsong = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 3).data().toInt();

    if (currentSubsong < 1) {
        currentSubsong = 1;
    }

    QFileInfo fileInfo(fullPath);
    QString suffix(fileInfo.suffix());
    QString baseName(fileInfo.baseName());

    setOutputDevice(outputDevice, baseName + subsong + "." + suffix);

    addDebugText("setOutputDevice with extra data: " + fullPath + "/" + baseName + subsong + "." + suffix);
    playStarted = true;

    if (!loadSound(fullPath, currentSubsong - 1)) {
        addDebugText("Error loading file.");
        updateButtons();
        return;
    }

    addDebugText("Check if we are playing already.");

    auto &sm = SoundManager::getInstance();

    if (sm.isPlaying()) {
        addDebugText("Stopping sound.");
        playAction->setText("Play");
        sm.stop();
    }

    addDebugText("Now playing");

    sm.playAudio(true);

    /*
     * when playing network streams, this setPosition invocation
     * prevents delays in displaying full track metadata
     */
    if (sm.info->plugin == PLUGIN_fmod && fileInfo.size() == 0) {
        sm.setPosition(0, FMOD_TIMEUNIT_MS);
    }

    int vol = ui->volumeSlider->value();
    sm.setVolume(static_cast<float>(vol) / 100);

    /* TODO
     * this workaround calculates the maximum pitch slider value (see ticket #623)
     * seems fmod can play at freqs slightly greater than 749700 (we should find the highest possible one)
     */
    float freq = sm.getNominalFrequency();
    int pitchSliderMaxValue = 749700 / freq * 100;
    ui->pitchSlider->setMaximum(pitchSliderMaxValue);
    sm.setFrequencyByMultiplier(ui->pitchSlider->value() / 100.0);

    addDebugText("Mute is:" + QString::number(isVolumeMuted));
    sm.setMute(isVolumeMuted);
    sm.pause(false);

    channels->updateChannels();

    getLength();

    ui->visualizer->init();
    ui->trackerView->init();

    PlaylistItem pi;
    pi.fullPath = fullPath;
    pi.info = sm.info;

    fileInfoParser->updateFileInfo(ui->tableInfo, &pi);

    QString title(fileInfo.fileName());

    if (sm.info->plugin == PLUGIN_fmod) {
        if (!pi.info->title.empty()) {
            title = pi.info->title.c_str();
        }
    } else {
        if (!pi.info->title.empty()) {
            title = fromUtf8OrLatin1(pi.info->title);
        } else if (!pi.info->containerFilenames.empty()) {
            title = fromUtf8OrLatin1(pi.info->containerLastFilename);
        }
    }

    QString artist = "";

    if (!pi.info->artist.empty()) {
        if (sm.info->plugin == PLUGIN_fmod) {
            artist = pi.info->artist.c_str();
        } else {
            artist = fromUtf8OrLatin1(pi.info->artist);
        }
    } else if (!pi.info->author.empty()) {
        if (sm.info->plugin == PLUGIN_fmod) {
            artist = pi.info->author.c_str();
        } else {
            artist = fromUtf8OrLatin1(pi.info->author);
        }
    } else if (!pi.info->composer.empty()) {
        if (sm.info->plugin == PLUGIN_fmod) {
            artist = pi.info->composer.c_str();
        } else {
            artist = fromUtf8OrLatin1(pi.info->composer);
        }
    }

    if (artist != "") {
        ui->labelFilename->setText(artist + " - " + title);
        windowTitle = artist + " - " + title + " - " + PROJECT_NAME;
    } else {
        ui->labelFilename->setText(title);
        windowTitle = title + " - " + PROJECT_NAME;
    }

    if (isMinimized() || !this->isVisible()) {
        this->setWindowTitle(windowTitle);
    } else {
        this->setWindowTitle(PROJECT_NAME_VERSIONED);
    }

    tray->setToolTip(windowTitle);

    QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 0, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, title, Qt::EditRole);

    index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 1, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(
        index, sm.info->fileFormat.c_str(), Qt::EditRole);
    index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 2, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(
        index, msToNiceStringExact(songLengthMs, displayMilliseconds), Qt::EditRole);
    index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 5, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, songLengthMs, Qt::EditRole);
    index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 8, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, artist, Qt::EditRole);

    index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 6, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index, false, Qt::EditRole);

    currentSubsong = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 3).data().toInt();

    QModelIndex index2 = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 0);
    tableWidgetPlaylists[currentPlaylist]->scrollTo(index2);

    // set text of currently playing sound to main color
    highlightPlaylistItem(currentPlaylist, rowProvided);

    // set text of currently playing playlist to main color
    QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
    l.at(0)->setForeground(QColor(colorMain.left(7)));

    ui->positionSlider->setEnabled(sm.info->getSeekable());

    updateSamplesAndInstruments();

    // ok, now to add all subsongs, one for each row

    if (tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 3).data().toInt() < 1 &&
        sm.info->numSubsongs > 1) {
        QModelIndex index = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 3, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index, 1, Qt::EditRole);

        currentSubsong = 1;

        QString title = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 0).data().toString();
        QString fileFormat = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 1).data().toString();
        QString fullpath = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 4).data().toString();
        QString artist = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 8).data().toString();

        for (int i = 1; i < sm.info->numSubsongs; i++) {
            addPlaylistEntry(tableWidgetPlaylists[currentPlaylist], rowProvided + i, title, fileFormat, "", i + 1,
                             fullpath, 0, artist);
        }
    }

    QModelIndex index3 = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 7, QModelIndex());
    tableWidgetPlaylists[currentPlaylist]->model()->setData(index3, true, Qt::EditRole);
    tableWidgetPlaylists[currentPlaylist]->update();

    QString printerText = tableWidgetPlaylists[currentPlaylist]->model()->index(rowProvided, 0).data().toString();
    //        if(printerText.isEmpty())
    //        {
    //            printerText=sm.info->filename.c_str();
    //        }

    updateScrollText();

    ui->visualizer->getEffect()->setPrinterText(printerText);

    updateButtons();
}

void MainWindow::updateSamplesAndInstruments() const {
    ui->instruments->clear();
    ui->instruments->setColumnCount(0);
    ui->instruments->setRowCount(0);
    ui->instruments->horizontalHeader()->setStretchLastSection(false);

    ui->samples->clear();
    ui->samples->setColumnCount(0);
    ui->samples->setRowCount(0);
    ui->samples->horizontalHeader()->setStretchLastSection(false);

    const auto &info = SoundManager::getInstance().info;

    if (info == nullptr) {
        return;
    }

    switch (info->plugin) {
        case PLUGIN_adplug:
            updateInstrumentsAdplug(info);
            break;
        case PLUGIN_asap:
            updateInstrumentsAsap(info);
            break;
        case PLUGIN_flod:
            updateSamplesFlod(info);
            break;
        case PLUGIN_hivelytracker:
            updateInstrumentsHivelytracker(info);
            break;
        case PLUGIN_klystron:
            updateInstrumentsKlystron(info);
            break;
        case PLUGIN_libopenmpt:
            updateSamplesLibopenmpt(info);
            updateInstrumentsLibopenmpt(info);
            break;
        case PLUGIN_libpac:
            updateSamplesLibpac(info);
            break;
        case PLUGIN_libxmp:
            updateSamplesLibxmp(info);
            updateInstrumentsLibxmp(info);
            break;
        case PLUGIN_protrekkr:
            updateSamplesProtrekkr(info);
            updateInstrumentsProtrekkr(info);
            break;
        case PLUGIN_uade:
            updateSamplesUade(info);
            break;
        case PLUGIN_zxtune:
            updateSamplesZxtune(info);
            break;
        default: ;
    }
}

void MainWindow::updateInstrumentsAdplug(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name");

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateInstrumentsAsap(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name");

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateSamplesFlod(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Volume");

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
    ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
    ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
    ui->samples->setColumnWidth(1, sampleColumnNameWidth);
    ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
    ui->samples->setColumnWidth(3, sampleColumnVolumeWidth);
    ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));
        ui->samples->setItem(j, 2, new QTableWidgetItem(QString::number(info->samplesSize[j])));
        ui->samples->setItem(j, 3, new QTableWidgetItem(QString::number(info->samplesVolume[j])));
    }
}

void MainWindow::updateInstrumentsHivelytracker(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name") << tr("Volume") << tr("Wavelen");

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->setColumnWidth(2, instrumentColumnVolumeWidth);
    ui->instruments->setColumnWidth(3, instrumentColumnWaveLengthWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
        ui->instruments->setItem(j, 2, new QTableWidgetItem(QString::number(info->instrumentsVolume[j])));
        ui->instruments->setItem(j, 3, new QTableWidgetItem(QString::number(info->instrumentsWavelen[j])));
    }
}

void MainWindow::updateInstrumentsKlystron(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name");

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateInstrumentsLibopenmpt(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name");
    //<< tr("Volume") << tr("Wavelen") << tr("Attack") << tr("Attack Volume") << tr("Decay") << tr("Decay Volume")
    //<< tr("Sustain") << tr("Sustain Volume") << tr("Release") << tr("Release Volume") << tr("Delay")
    //<< tr("Delay Depth") << tr("Delay Speed") << tr("Filter Lower Limit") << tr("Filter Upper Limit")
    //<< tr("Filterspeed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed")

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateSamplesLibopenmpt(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend")
            << tr("Volume") << tr("Finetune");
    //<< tr("Resolution") << tr("Channels")

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
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
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));

        if (info->samplesSize[j] > 0) {
            ui->samples->setItem(j, 2, new QTableWidgetItem(QString::number(info->samplesSize[j])));
            ui->samples->setItem(j, 3, new QTableWidgetItem(QString::number(info->samplesLoopStart[j])));
            ui->samples->setItem(j, 4, new QTableWidgetItem(QString::number(info->samplesLoopEnd[j])));
            ui->samples->setItem(j, 5, new QTableWidgetItem(QString::number(info->samplesVolume[j])));
            ui->samples->setItem(j, 6, new QTableWidgetItem(QString::number(info->samplesFineTune[j])));
        }
    }
}

void MainWindow::updateSamplesLibpac(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend")
            << tr("Volume") << tr("Finetune") << tr("Resolution");

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
    ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
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
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));
        ui->samples->setItem(j, 7, new QTableWidgetItem(info->samples16Bit[j] ? "16-bit" : "8-bit"));
        ui->samples->setItem(j, 2, new QTableWidgetItem(QString::number(info->samplesSize[j])));
        ui->samples->setItem(j, 3, new QTableWidgetItem(QString::number(info->samplesLoopStart[j])));
        ui->samples->setItem(j, 4, new QTableWidgetItem(QString::number(info->samplesLoopEnd[j])));
        ui->samples->setItem(j, 5, new QTableWidgetItem(QString::number(info->samplesVolume[j])));
        ui->samples->setItem(j, 6, new QTableWidgetItem(QString::number(info->samplesFineTune[j])));
    }
}

void MainWindow::updateInstrumentsLibxmp(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name") << tr("Volume") << tr("Wavelen");
    //<< tr("Attack") << tr("Attack Volume") << tr("Decay") << tr("Decay Volume") << tr("Sustain")
    //<< tr("Sustain Volume") << tr("Release") << tr("Release Volume") << tr("Delay") << tr("Delay Depth")
    //<< tr("Delay Speed") << tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed")
    //<< tr("Filter Lower Limit") << tr("Filter Upper Limit") << tr("Filterspeed")

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->setColumnWidth(2, instrumentColumnVolumeWidth);
    ui->instruments->setColumnWidth(3, instrumentColumnWaveLengthWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateSamplesLibxmp(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Loopstart") << tr("Loopend")
            << tr("Volume") << tr("Finetune");
    //<< tr("Resolution") << tr("Channels")

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
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
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));

        if (info->samplesSize[j] > 0) {
            ui->samples->setItem(j, 2, new QTableWidgetItem(QString::number(info->samplesSize[j])));
            ui->samples->setItem(j, 3, new QTableWidgetItem(QString::number(info->samplesLoopStart[j])));
            ui->samples->setItem(j, 4, new QTableWidgetItem(QString::number(info->samplesLoopEnd[j])));

            // TODO these two probably should only be divided for mods (not xm etc.)
            ui->samples->setItem(j, 5, new QTableWidgetItem(QString::number(info->samplesVolume[j] / 4)));
            ui->samples->setItem(j, 6, new QTableWidgetItem(QString::number(info->samplesFineTune[j] / 16)));
        }
    }
}

void MainWindow::updateInstrumentsProtrekkr(const Info *info) const {
    if (info->numInstruments <= 0) {
        return;
    }

    QStringList columnLabelsInstruments;
    columnLabelsInstruments << tr("#") << tr("Name");

    ui->instruments->setColumnCount(static_cast<int>(columnLabelsInstruments.size()));
    ui->instruments->setHorizontalHeaderLabels(columnLabelsInstruments);
    ui->instruments->setColumnWidth(0, instrumentColumnNumberWidth);
    ui->instruments->setColumnWidth(1, instrumentColumnNameWidth);
    ui->instruments->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->instruments->setRowCount(info->numInstruments);

    for (int j = 0; j < info->numInstruments; j++) {
        ui->instruments->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->instruments->setItem(j, 1, new QTableWidgetItem(info->instruments[j].c_str()));
    }
}

void MainWindow::updateSamplesProtrekkr(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name");

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
    ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
    ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
    ui->samples->setColumnWidth(1, sampleColumnNameWidth);
    ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));
    }
}

void MainWindow::updateSamplesUade(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name") << tr("Size") << tr("Volume");

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
    ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
    ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
    ui->samples->setColumnWidth(1, sampleColumnNameWidth);
    ui->samples->setColumnWidth(2, sampleColumnSizeWidth);
    ui->samples->setColumnWidth(3, sampleColumnVolumeWidth);
    ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft);
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));

        if (info->samplesSize[j] > 0) {
            ui->samples->setItem(j, 2, new QTableWidgetItem(QString::number(info->samplesSize[j])));
            ui->samples->setItem(j, 3, new QTableWidgetItem(QString::number(info->samplesVolume[j])));
        }
    }
}

void MainWindow::updateSamplesZxtune(const Info *info) const {
    if (info->numSamples <= 0) {
        return;
    }

    QStringList columnLabelsSamples;
    columnLabelsSamples << tr("#") << tr("Name");

    ui->samples->setColumnCount(static_cast<int>(columnLabelsSamples.size()));
    ui->samples->setHorizontalHeaderLabels(columnLabelsSamples);
    ui->samples->setColumnWidth(0, sampleColumnNumberWidth);
    ui->samples->setColumnWidth(1, sampleColumnNameWidth);
    ui->samples->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->samples->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->samples->setRowCount(info->numSamples);

    for (int j = 0; j < info->numSamples; j++) {
        ui->samples->setItem(j, 0, new QTableWidgetItem(QString::number(j + 1)));
        ui->samples->setItem(j, 1, new QTableWidgetItem(info->samples[j].c_str()));
    }
}

void MainWindow::restoreLayout() {
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
    dockManager->restoreState(dockingState);
}

void MainWindow::exportInstrumentToWav() {
    exportInstrument("WAV");
}

void MainWindow::muteAllChannels() const {
    channels->muteAllChannels();
}

void MainWindow::unmuteAllChannels() const {
    channels->unmuteAllChannels();
}

void MainWindow::exportInstrumentToIff() {
    exportInstrument("IFF");
}

void MainWindow::exportInstrument(const QString &format) {
    static auto regex = QRegularExpression(":|;|/|\"");

    const auto &info = SoundManager::getInstance().info;
    const int row = ui->samples->currentRow();

    if (info->plugin != PLUGIN_libxmp || ui->samples->rowCount() <= 0 || info->samplesSize[row] <= 0) {
        return;
    }

    QString defaultFileName = info->samples[row].c_str();
    defaultFileName.replace(regex, "-");

    const unsigned int loopStart = info->samplesLoopStart[row];
    const unsigned int loopLength = info->samplesLoopEnd[row] - info->samplesLoopStart[row];

    QString filter;

    if (format == "WAV") {
        filter = "Wave (*.wav)";
    } else if (format == "IFF") {
        filter = "IFF (*.iff)";
    } else return;

    const QString fileName = QFileDialog::getSaveFileName(this, "Export sample", "/" + defaultFileName,
                                                          filter);

    if (fileName.isEmpty()) return;

    if (format == "WAV") {
        addDebugText(
            "Saving sample to WAV, no. " + QString::number(row) + ", filesize: " + QString::number(
                info->samplesSize[row]));

        typedef struct wavHeader_t {
            uint32_t chunkID, chunkSize, format, subchunk1ID, subchunk1Size;
            uint16_t audioFormat, numChannels;
            uint32_t sampleRate, byteRate;
            uint16_t blockAlign, bitsPerSample;
            uint32_t subchunk2ID, subchunk2Size;
        } wavHeader_t;

        typedef struct sampleLoop_t {
            uint32_t dwIdentifier, dwType, dwStart;
            uint32_t dwEnd, dwFraction, dwPlayCount;
        } sampleLoop_t;

        typedef struct samplerChunk_t {
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
        wavHeader.subchunk2Size = info->samplesSize[row];
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
            samplerChunk.loop.dwEnd = loopStart + loopLength - 1;
        }

        FILE *pFile = fopen(fileName.toStdString().c_str(), "wb");
        fwrite(&wavHeader, sizeof(char), sizeof wavHeader, pFile);

        for (int i = 0; i < info->samplesSize[row]; i++)
            fputc(static_cast<uint8_t>(info->samplesData[row][i] + 128), pFile);

        if (info->samplesSize[row] & 1)
            fputc(0, pFile); // pad align byte

        if (loopStart + loopLength > 2) // loop enabled?
            fwrite(&samplerChunk, sizeof (samplerChunk), 1, pFile);

        fclose(pFile);
    } else if (format == "IFF") {
        addDebugText(
            "Saving sample to IFF, no. " + QString::number(row) + ", filesize: " + QString::number(
                info->samplesSize[row]));

        const QString fileName =
                QFileDialog::getSaveFileName(this, "Export sample", "/" + defaultFileName, "IFF (*.iff)");

        FILE *f = fopen(fileName.toStdString().c_str(), "wb");

        // "FORM" chunk
        iffWriteChunkHeader(f, "FORM", 0); // "FORM" chunk size is overwritten later
        iffWriteUint32(f, 0x38535658); // "8SVX"

        // "VHDR" chunk
        iffWriteChunkHeader(f, "VHDR", 20);

        if (loopStart + loopLength > 2) // loop enabled?
        {
            iffWriteUint32(f, loopStart); // oneShotHiSamples
            iffWriteUint32(f, loopLength); // repeatHiSamples
        } else {
            iffWriteUint32(f, 0); // oneShotHiSamples
            iffWriteUint32(f, 0); // repeatHiSamples
        }

        iffWriteUint32(f, 0); // samplesPerHiCycle
        iffWriteUint16(f, 16574); // samplesPerSec
        iffWriteUint8(f, 1); // ctOctave (number of samples)
        iffWriteUint8(f, 0); // sCompression
        //iffWriteUint32(f, s->volume * 1024); // volume (max: 65536/0x10000)
        iffWriteUint32(f, 1 * 1024); // volume (max: 65536/0x10000)

        // "NAME" chunk
        auto chunkLen = static_cast<uint32_t>(strlen(info->samples[row].c_str()));

        if (chunkLen > 0) {
            iffWriteChunkHeader(f, "NAME", chunkLen);
            iffWriteChunkData(f, info->samples[row].c_str(), chunkLen);
        }

        // "ANNO" chunk (we put the program name here)
        constexpr char annoStr[] = PROJECT_NAME;
        chunkLen = sizeof (annoStr) - 1;
        iffWriteChunkHeader(f, "ANNO", chunkLen);
        iffWriteChunkData(f, annoStr, chunkLen);

        // "BODY" chunk
        chunkLen = info->samplesSize[row];
        iffWriteChunkHeader(f, "BODY", chunkLen);
        iffWriteChunkData(f, info->samplesData[row], chunkLen);

        // go back and fill in "FORM" chunk size
        chunkLen = ftell(f) - 8;
        fseek(f, 4, SEEK_SET);
        iffWriteUint32(f, chunkLen);

        fclose(f);
    }
}

void MainWindow::fullscreenVisualizer() const {
    visualizerFullScreen->showFullScreen();
}

void MainWindow::showNextVisualizer() const {
    visualizerFullScreen->showFullScreen();
}

void MainWindow::fullscreenTracker() const {
    trackerFullScreen->showFullScreen();
}

void MainWindow::selectAllLogWindow() const {
    ui->Debug->selectAll();
}

void MainWindow::copyLogWindow() const {
    ui->Debug->copy();
}

void MainWindow::clearLogWindow() const {
    ui->Debug->clear();
}

void MainWindow::renamePlaylist() {
    if (ui->listWidget->currentItem()->text() == PLAYLIST_DEFAULT_FILENAME) return;

    bool ok;
    const QString oldName = ui->listWidget->currentItem()->text();

    if (QString newName = QInputDialog::getText(this, "Rename Playlist", "New name:", QLineEdit::Normal, oldName, &ok);
        oldName != newName && ok && !newName.isEmpty()) {
        if (!newName.endsWith(PLAYLIST_DEFAULT_EXTENSION)) {
            newName = newName + PLAYLIST_DEFAULT_EXTENSION;
        }

        const QString newOrgFilename = newName;
        int suffix = 0;

        while (tableWidgetPlaylists.contains(newName)) {
            suffix++;
            newName = newOrgFilename + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
        }

        const QString playlistNewName = userPath + PLAYLISTS_DIR + QDir::separator() + newName;
        const QString playlistOldName = userPath + PLAYLISTS_DIR + QDir::separator() + oldName;
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
        dockWidgetPlaylist->setWindowTitle(newName);
    }
}

void MainWindow::savePlaylistAs() {
    bool ok;
    const QString oldName = ui->listWidget->currentItem()->text();

    QString newName = QInputDialog::getText(this, "Save Playlist As",
                                            "New name:", QLineEdit::Normal,
                                            oldName, &ok);

    if (oldName == newName || !ok || newName.isEmpty()) return;

    if (!newName.endsWith(PLAYLIST_DEFAULT_EXTENSION)) {
        newName = newName + PLAYLIST_DEFAULT_EXTENSION;
    }

    const QString newOrgFilename = newName;
    int suffix = 0;

    while (tableWidgetPlaylists.contains(newName)) {
        suffix++;
        newName = newOrgFilename + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
    }

    const QString playlistNewName = userPath + PLAYLISTS_DIR + QDir::separator() + newName;
    const QString playlistOldName = userPath + PLAYLISTS_DIR + QDir::separator() + oldName;

    savePlayList(playlistOldName, playlistNewName);

    addDebugText("Saving playlist: " + playlistOldName + " as " + playlistNewName);
    QListWidgetItem *newItem = ui->listWidget->currentItem()->clone();
    newItem->setText(newName);
    newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));

    ui->listWidget->addItem(newItem);

    const auto item = new MyItemDelegate(this);

    item->setMainColor(QColor(colorMain.left(7)));
    ui->listWidget->setItemDelegate(item);

    const auto tv = new DraggableTableView();

    tv->setDragBackgroundColor(QColor(colorMain.left(7)));
    tv->setDragTextColor(QColor(colorMainText.left(7)));
    tv->setupDelegate(); // has to be called after colors are set

    const auto pm = new PlaylistModel(this);
    const auto proxyModel = new QSortFilterProxyModel(pm); // create proxy

    proxyModel->setSourceModel(pm);
    tv->setModel(proxyModel);

    tv->setColumnHidden(4, true);
    tv->setColumnHidden(5, true);
    tv->setColumnHidden(6, true);
    tv->setColumnHidden(7, true);

    tableWidgetPlaylists[newName] = tv;

    const QFont roboto("Roboto");

    tableWidgetPlaylists[newName]->setStyleSheet(
        ui->dockWidgetContents_4->styleSheet() +
        "QHeaderView::section{font-family:Roboto;padding:0;} QTableView{padding:9px;}");

    tableWidgetPlaylists[newName]->setFont(roboto);
    tableWidgetPlaylists[newName]->installEventFilter(this);

    swapColumns(tableWidgetPlaylists[newName]);

    connect(tableWidgetPlaylists[newName], SIGNAL(doubleClicked(const QModelIndex &)),
            SLOT(on_playlist_itemDoubleClicked(const QModelIndex &)));

    const QUrl u = QUrl::fromLocalFile(QDir::separator() + userPath + PLAYLISTS_DIR + "/" + newName);
    QList<QUrl> ql;
    ql.append(u);

    addSong(ql, 0, newName, false);

    createThePopupMenuCurrentPlaylist(newName);
}

void MainWindow::savePlayList(QString path, QString newPath) {
    const QFileInfo fileInfoOld(path);
    QFile file(newPath);

    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&file);

    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);

    out << "#EXTM3U\n";
    addDebugText("Saving playlist: " + newPath);

    for (int i = 0; i < tableWidgetPlaylists[fileInfoOld.fileName()]->model()->rowCount(); i++) {
        QString playlistKey = fileInfoOld.fileName();
        QString path = tableWidgetPlaylists[playlistKey]->model()->index(i, 4).data().toString();
        QString extInfo = "#EXTINF:" +
                          QString::number(tableWidgetPlaylists[playlistKey]->model()->index(i, 5).data().toInt()) + ","
                          + QString(
                              tableWidgetPlaylists[playlistKey]->model()->index(i, 0).data().toString());
        out << extInfo << "\n" << path << NEZPLAYLISTSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 1)
                .data().toString() << PLAYLISTFIELDSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 3).
                data().toString() << PLAYLISTFIELDSPLITTER << tableWidgetPlaylists[playlistKey]->model()->index(i, 0).
                data()
                .toString() << PLAYLISTFIELDSPLITTER << "[time(h:m:s)] " << PLAYLISTFIELDSPLITTER << "[loop(h:m:s)][-]"
                <<
                PLAYLISTFIELDSPLITTER << "[fade(h:m:s)]" << PLAYLISTFIELDSPLITTER << "[loopcount]" <<
                PLAYLISTFIELDSPLITTER
                << tableWidgetPlaylists[playlistKey]->model()->index(i, 8).data().toString() << "\n";
    }

    file.close();
}

void MainWindow::savePlaylist() {
    savePlayList(userPath + PLAYLISTS_DIR + "/" + ui->listWidget->currentItem()->text(),
                 userPath + PLAYLISTS_DIR + "/" + ui->listWidget->currentItem()->
                 text());
}

void MainWindow::deleteAllPlaylists() {
    for (int rowNumber = ui->listWidget->count() - 1; rowNumber >= 0; rowNumber--) {
        ui->listWidget->setCurrentRow(rowNumber);

        if (ui->listWidget->currentItem()->text() != PLAYLIST_DEFAULT_FILENAME) {
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

void MainWindow::deletePlaylist() {
    if (ui->listWidget->currentItem()->text() == PLAYLIST_DEFAULT_FILENAME) return;

    int rowNumber = ui->listWidget->currentRow();
    rowNumber--;

    const QString playlistToDelete = userPath + PLAYLISTS_DIR + QDir::separator() + ui->listWidget->currentItem()->
                                     text();
    addDebugText("Deleting playlist " + playlistToDelete);
    QFile::remove(playlistToDelete);
    //playlists.remove(ui->listWidget->currentItem()->text());
    tableWidgetPlaylists.remove(ui->listWidget->currentItem()->text());
    delete ui->listWidget->takeItem(ui->listWidget->currentRow());
    ui->listWidget->setCurrentRow(rowNumber);
    on_listWidget_itemClicked(ui->listWidget->item(rowNumber));
}

void MainWindow::clearPlaylist() {
    addDebugText("Clears playlist " + selectedPlaylist);
    removeHighlight();
    currentRow = 0;
    tableWidgetPlaylists[selectedPlaylist]->model()->removeRows(
        0, tableWidgetPlaylists[selectedPlaylist]->model()->rowCount());

    on_buttonStop_clicked();
    isUpdateCurrentRowToNextEnabled = false;

    if (selectedPlaylist == currentPlaylist) {
        resetShuffle(currentPlaylist);
    }
}

void MainWindow::showContainingFolder() {
    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows()) {
        QFile file(tableWidgetPlaylists[selectedPlaylist]->model()->index(idx.row(), 4).data().toString());

        if (QFileInfo fileinfo(file);
            !fileinfo.path().startsWith("http://", Qt::CaseInsensitive) && !fileinfo.path().startsWith("https://")) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.path()));
        }
    }
}

void MainWindow::deleteFilesInPlaylist() {
    bool rowDeleted = false;
    QVector<int> selectedRowsIdx;

    addDebugText("Removing items from playlist: " + selectedPlaylist);

    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows()) {
        selectedRowsIdx.append(idx.row());
        addDebugText("Removing items from playlist: " + QString::number(idx.row()));
    }

    ranges::sort(selectedRowsIdx);

    const int currentRowPreDelete = currentRow;

    for (int idx = selectedRowsIdx.size() - 1; idx >= 0; idx--) {
        tableWidgetPlaylists[selectedPlaylist]->model()->removeRow(selectedRowsIdx.at(idx));
        rowDeleted = true;

        if (selectedRowsIdx.at(idx) < currentRow && currentRow != 0) {
            currentRow--;
        }

        addDebugText("Removing row: " + QString::number(selectedRowsIdx.at(idx)));
    }

    if (rowDeleted) {
        addDebugText("Row(s) in playlist deleted.");

        if (tableWidgetPlaylists[selectedPlaylist]->model()->rowCount() == 0) {
            on_buttonStop_clicked();
            isUpdateCurrentRowToNextEnabled = false;
        } else {
            for (int idx = selectedRowsIdx.size() - 1; idx >= 0; idx--) {
                if (selectedRowsIdx.at(idx) == currentRowPreDelete) {
                    on_buttonStop_clicked();
                    isUpdateCurrentRowToNextEnabled = false;
                    break;
                }
            }
        }

        if (selectedPlaylist == currentPlaylist) {
            resetShuffle(currentPlaylist);
        }
    }
}

void MainWindow::deleteFilesInvertedInPlaylist() {
    bool rowDeleted = false;

    addDebugText("Removing items from playlist: " + selectedPlaylist);

    // get all selected rows
    QVector<int> selectedRowsIdx;

    foreach(QModelIndex idx, tableWidgetPlaylists[selectedPlaylist]->selectionModel()->selectedRows()) {
        selectedRowsIdx.append(idx.row());
    }

    // get all NOT selected rows
    QVector<int> notSelectedRowsIdx;

    for (int i = 0; i < tableWidgetPlaylists[selectedPlaylist]->model()->rowCount(); i++) {
        if (!selectedRowsIdx.contains(i)) {
            notSelectedRowsIdx.append(i);
        }
    }

    for (int idx = notSelectedRowsIdx.size() - 1; idx >= 0; idx--) {
        tableWidgetPlaylists[selectedPlaylist]->model()->removeRow(notSelectedRowsIdx.at(idx));
        rowDeleted = true;

        if (currentRow >= notSelectedRowsIdx.at(idx) && currentRow != 0) {
            currentRow--;
        }
    }

    if (rowDeleted) {
        addDebugText("Row(s) in playlist deleted.");

        if (selectedPlaylist == currentPlaylist) {
            resetShuffle(currentPlaylist);
        }
    }
}

void MainWindow::openSettings() {
    settingsWindow settingsWindow(this);
    settingsWindow.exec();
}

void MainWindow::addFolder() {
    const auto root = QUrl(lastDir);

    if (const QUrl u = QFileDialog::getExistingDirectoryUrl(this, "Add folder", root);
        !u.isEmpty()) {
        QList<QUrl> ql;
        ql.append(u);
        addSong(ql, 0, ui->listWidget->currentItem()->text(), true);
        lastDir = u.toLocalFile();
    }
}

void MainWindow::addFiles() {
    const QString root = lastDir;

    if (QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add files", root, tr("All files (*.*)"));
        !fileNames.empty()) {
        addSong(fileNames, 0, selectedPlaylist, true);
        lastDir = QFileInfo(fileNames.last()).absolutePath();
    }
}

void MainWindow::setChannelEnabled(const int index, const bool enable) const {
    channels->setChannelEnabled(index, enable);
}

bool MainWindow::isChannelEnabled(const int index) const {
    return channels->getChannelEnabled(index);
}

void MainWindow::createThePopupLogWindow() {
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

// creates the popup menu for playlists window
void MainWindow::createThePopupMenuPlaylists() {
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
    QListWidget *list = ui->listWidget;
    connect(list, &QListWidget::customContextMenuRequested, [list, this](const QPoint &pos) {
        const QModelIndex index = list->indexAt(pos);

        if (list->itemAt(pos) != nullptr) {
            if (index.row() == 0) {
                QMenu rightClickMenu(list);
                rightClickMenu.addActions({
                    clearPlaylistAction, deleteAllPlaylistsAction, savePlaylistAction, savePlaylistAsAction
                });
                rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
            } else {
                QMenu rightClickMenu(list);
                rightClickMenu.addActions({
                    renamePlaylistAction, clearPlaylistAction, deletePlaylistAction, deleteAllPlaylistsAction,
                    savePlaylistAction, savePlaylistAsAction
                });
                rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
            }
        } else {
            QMenu rightClickMenu(list);
            rightClickMenu.addActions({deleteAllPlaylistsAction});
            rightClickMenu.exec(list->viewport()->mapToGlobal(pos));
        }
    });
}

void MainWindow::createThePopupMenuVisualizer() {
    fullscreenVisualizerAction = new QAction(tr("&Show Fullscreen"), this);

    showNextVisualizerAction = new QAction(tr("S&how Next Visualizer"), this);

    connect(fullscreenVisualizerAction, SIGNAL(triggered()), this, SLOT(fullscreenVisualizer()));
    connect(showNextVisualizerAction, SIGNAL(triggered()), this, SLOT(showNextVisualizer()));

    ui->visualizer->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->visualizer->addActions({fullscreenVisualizerAction});
}

void MainWindow::createThePopupMenuTracker() {
    fullscreenTrackerAction = new QAction(tr("&Show Fullscreen"), this);

    connect(fullscreenTrackerAction, SIGNAL(triggered()), this, SLOT(fullscreenTracker()));

    ui->trackerView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->trackerView->addActions({fullscreenTrackerAction});
}

// creates the popup menu for current playlist window
void MainWindow::createThePopupMenuCurrentPlaylist(const QString &playlist) {
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

void MainWindow::createThePopupMenuInstruments() {
    exportInstrumentWavAction = new QAction(tr("&Export to .wav"), this);
    exportInstrumentIffAction = new QAction(tr("&Export to .iff"), this);

    connect(exportInstrumentWavAction, SIGNAL(triggered()), this, SLOT(exportInstrumentToWav()));
    connect(exportInstrumentIffAction, SIGNAL(triggered()), this, SLOT(exportInstrumentToIff()));

    ui->samples->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->samples->addActions({exportInstrumentWavAction});
    ui->samples->addActions({exportInstrumentIffAction});
}

void MainWindow::createThePopupMenuChannels() {
    muteAllChannelsAction = new QAction(tr("&Mute All Channels"), this);
    unmuteAllChannelsAction = new QAction(tr("&Unmute All Channels"), this);

    connect(muteAllChannelsAction, SIGNAL(triggered()), this, SLOT(muteAllChannels()));
    connect(unmuteAllChannelsAction, SIGNAL(triggered()), this, SLOT(unmuteAllChannels()));

    ui->dockWidgetContents_7->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->dockWidgetContents_7->addActions({muteAllChannelsAction});
    ui->dockWidgetContents_7->addActions({unmuteAllChannelsAction});
}

int MainWindow::getCurrentRow() const {
    return currentRow;
}

void MainWindow::setCurrentRow(const int row) {
    currentRow = row;
}

void MainWindow::playPrevSong() {
    addDebugText("Play previous song.");

    if ((currentRow != 0 && !isShuffleEnabled()) || (isShuffleEnabled() && currentShufflePosition[currentPlaylist] > 0)
        ||
        (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked && !isShuffleEnabled())) {
        if (isShuffleEnabled()) {
            currentShufflePosition[currentPlaylist]--;
            removeHighlight();
            currentRow = shufflePlayed[currentPlaylist].at(currentShufflePosition[currentPlaylist]);
        } else if (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked && currentRow == 0) {
            removeHighlight();
            currentRow = tableWidgetPlaylists[currentPlaylist]->model()->rowCount() - 1;
        } else {
            removeHighlight();
            currentRow--;
        }

        playSongAtRow(currentRow);
        isUpdateCurrentRowToNextEnabled = true;
    }
}

void MainWindow::on_buttonNext_clicked() {
    buttonNextClicked = true;
    playNextSong(true);
}

void MainWindow::on_buttonPrev_clicked() {
    buttonNextClicked = false;
    playPrevSong();
}

void MainWindow::setOutputDeviceSetting(const int outputDeviceProvided) {
    outputDevice = outputDeviceProvided;
}

void MainWindow::setOutputDevice(const int outputDeviceProvided, const QString &fullPath) {
    auto &sm = SoundManager::getInstance();

    sm.shutdown();
    sm.Init(outputDeviceProvided, fullPath);

    sm.setNormalizeEnabled(normalizeEnabled);
    sm.setNormalizeFadeTime(normalizeFadeTime);
    sm.setNormalizeMaxAmp(normalizeMaxAmp);
    sm.setNormalizeThreshold(normalizeThreshold);
    sm.setReverbPreset(reverbPreset);
    sm.setReverbEnabled(reverbEnabled);

    resetAll();

    outputDevice = outputDeviceProvided;
}

int MainWindow::getOutputDevice() const {
    return outputDevice;
}

void MainWindow::setResetVolume(const bool resetVolumeProvided) {
    resetVolume = resetVolumeProvided;
}

bool MainWindow::getResetVolume() const {
    return resetVolume;
}

void MainWindow::setDefaultPlayMode(const int defaultPlayModeProvided) {
    defaultPlayMode = defaultPlayModeProvided;
}

int MainWindow::getDefaultPlayMode() const {
    return defaultPlayMode;
}

void MainWindow::setReverbPreset(const QString &name) {
    SoundManager::getInstance().setReverbPreset(name);
    reverbPreset = name;
}

void MainWindow::setIgnoreSuffix(const QString &suffix) {
    ignoreSuffix = suffix;
}

void MainWindow::setIgnorePrefix(const QString &prefix) {
    ignorePrefix = prefix;
}

QString MainWindow::getReverbPreset() const {
    return reverbPreset;
}

QString MainWindow::getIgnorePrefix() const {
    return ignorePrefix;
}

QString MainWindow::getIgnoreSuffix() const {
    return ignoreSuffix;
}

void MainWindow::setReverbEnabled(const bool reverb) {
    reverbEnabled = reverb;
    SoundManager::getInstance().setReverbEnabled(reverb);
}

bool MainWindow::isReverbEnabled() const {
    return reverbEnabled;
}

void MainWindow::setNormalizeEnabled(const bool normalize) {
    normalizeEnabled = normalize;
    SoundManager::getInstance().setNormalizeEnabled(normalize);
}

bool MainWindow::isNormalizeEnabled() const {
    return normalizeEnabled;
}

bool MainWindow::getDisplayMilliseconds() const {
    return displayMilliseconds;
}

bool MainWindow::getEnqueueItems() const {
    return enqueueItems;
}

bool MainWindow::getShowCheckBoxLoopPoints() const {
    return isShownCheckBoxLoopPoints;
}

bool MainWindow::getSystrayChecked() const {
    return isSystrayChecked;
}

bool MainWindow::getSystrayOnMinimizeChecked() const {
    return isSystrayOnMinimizeChecked;
}

bool MainWindow::getMenuBarHiddenChecked() const {
    return isMenuBarHiddenChecked;
}

void MainWindow::setNormalizeFadeTime(const int fadeTime) {
    normalizeFadeTime = fadeTime;
    SoundManager::getInstance().setNormalizeFadeTime(fadeTime);
}

int MainWindow::getNormalizeFadeTime() const {
    return normalizeFadeTime;
}

void MainWindow::setNormalizeThreshold(const int threshold) {
    SoundManager::getInstance().setNormalizeThreshold(threshold);
    normalizeThreshold = threshold;
}

int MainWindow::getNormalizeThreshold() const {
    return normalizeThreshold;
}

void MainWindow::setNormalizeMaxAmp(const int maxAmp) {
    SoundManager::getInstance().setNormalizeMaxAmp(maxAmp);
    normalizeMaxAmp = maxAmp;
}

int MainWindow::getNormalizeMaxAmp() const {
    return normalizeMaxAmp;
}

int MainWindow::getResetVolumeValue() const {
    return resetVolumeValue;
}

int MainWindow::getPlaylistRowHeight() const {
    return playlistRowHeight;
}

QString MainWindow::getHvscSonglengthsPath() const {
    return hvscSonglengthsPath;
}

void MainWindow::setHvscSonglengthsPath(const QString &path) {
    hvscSonglengthsPath = path;
}

qint64 MainWindow::getBundledHvscSonglengthsDownloadEpoch() const {
    return bundledHvscSonglengthsDownloadEpoch;
}

QString MainWindow::getBundledHvscSonglengthsUpdateFrequency() const {
    return bundledHvscSonglengthsUpdateFrequency;
}

QString MainWindow::getBundledHvscSonglengthsPath() const {
    return bundledHvscSonglengthsPath;
}

void MainWindow::setBundledHvscSonglengthsPath(const QString &path) {
    bundledHvscSonglengthsPath = path;
}

int MainWindow::getPlaylistsRowHeight() const {
    return playlistsRowHeight;
}

int MainWindow::getNowPlayingFontSize() const {
    return nowPlayingFontSize;
}

void MainWindow::setResetVolumeValue(const int value) {
    resetVolumeValue = value;
}

void MainWindow::setSystrayChecked(const bool isChecked) {
    isSystrayChecked = isChecked;
    tray->setVisible(isChecked);
}

void MainWindow::setSystrayOnMinimizeChecked(const bool isChecked) {
    isSystrayOnMinimizeChecked = isChecked;
}

void MainWindow::setSystrayOnMinimizeEnabled(const bool isEnabled) {
    isSystrayOnMinimizeEnabled = isEnabled;
}

void MainWindow::setMenuBarHiddenChecked(const bool isChecked) {
    isMenuBarHiddenChecked = isChecked;
    menuBar()->setHidden(isChecked);
}

void MainWindow::saveSettings() const {
    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("outputDevice", outputDevice);
    settings.setValue("Internal/volume", ui->volumeSlider->value());
    settings.setValue("Internal/shuffle", ui->checkBoxShuffle->isChecked());
    settings.setValue("defaultPlayMode", defaultPlayMode);
    settings.setValue("Internal/playMode", defaultPlayMode == -1 ? playMode : defaultPlayMode);
    settings.setValue("defaultAudioLevel", resetVolume);
    settings.setValue("defaultAudioLevelValue", resetVolumeValue);
    settings.setValue("Internal/mute", ui->checkBoxVolumeOn->checkState() != Qt::Checked);
    settings.setValue("displayMilliseconds", displayMilliseconds);
    settings.setValue("enqueueItems", enqueueItems);
    settings.setValue("showLoopPoints", isShownCheckBoxLoopPoints);
    settings.setValue("normalizer", normalizeEnabled);
    settings.setValue("normalizerFadeTime", normalizeFadeTime);
    settings.setValue("normalizerThreshold", normalizeThreshold);
    settings.setValue("normalizerMaxAmp", normalizeMaxAmp);
    settings.setValue("reverb", reverbEnabled);
    settings.setValue("reverbPreset", reverbPreset);
    settings.setValue("systray", isSystrayChecked);
    settings.setValue("minimizeToSystray", isSystrayOnMinimizeChecked);
    settings.setValue("menuBarHidden", isMenuBarHiddenChecked);
    settings.setValue("Internal/selectedPlaylist", selectedPlaylist);
    settings.setValue("Internal/currentPlaylist", currentPlaylist);
    settings.setValue("Internal/currentRow", currentRow);
    settings.setValue("ignoreSuffixes", ignoreSuffix);
    settings.setValue("ignorePrefixes", ignorePrefix);
    settings.setValue("Appearance/colorMain", colorMain);
    settings.setValue("Appearance/colorMainHover", colorMainHover);
    settings.setValue("Appearance/colorMedium", colorMedium);
    settings.setValue("Appearance/colorBackground", colorBackground);
    settings.setValue("Appearance/colorBehindBackground", colorBehindBackground);
    settings.setValue("Appearance/colorMainText", colorMainText);
    settings.setValue("Appearance/colorSelection", colorSelection);
    settings.setValue("Appearance/colorButton", colorButton);
    settings.setValue("Appearance/colorButtonHover", colorButtonHover);
    settings.setValue("Appearance/colorDimmedText", colorDimmedText);
    settings.setValue("Visualizer/vuMeterColorTop", colorVisualizerTop);
    settings.setValue("Visualizer/vuMeterColorBottom", colorVisualizerBottom);
    settings.setValue("Visualizer/vuMeterColorMiddle", colorVisualizerMiddle);
    settings.setValue("Visualizer/colorBackground", colorVisualizerBackground);
    settings.setValue("Visualizer/vuMeterColorPeak", colorVisualizerPeak);
    settings.setValue("Visualizer/vuMeterPeaks", vuMeterPeaksEnabled);
    settings.setValue("Visualizer/vuMeterPeakHeight", vuMeterPeaksHeight);
    settings.setValue("Appearance/playlistItemRowHeight", playlistRowHeight);
    settings.setValue("Appearance/playlistRowHeight", playlistsRowHeight);
    settings.setValue("Appearance/nowPlayingFontSize", nowPlayingFontSize);
    settings.setValue("Visualizer/vuMeterWidth", getEffect()->getVuMeterWidth());
    settings.setValue("Visualizer/vuMeterOpacity", getEffect()->getVuMeterOpacity());
    settings.setValue("Visualizer/scrollerAmplitude", getEffect()->getAmplitude());
    settings.setValue("Visualizer/scrollerFrequency", getEffect()->getFrequency());
    settings.setValue("Visualizer/scrollerSinusSpeed", getEffect()->getSinusSpeed());
    settings.setValue("Visualizer/scrollerScrollSpeed", getEffect()->getScrollSpeed());
    settings.setValue("Visualizer/scrollerVerticalPosition", getEffect()->getVerticalScrollPosition());
    settings.setValue("Visualizer/scrollerFontXScale", getEffect()->getFontScaleX());
    settings.setValue("Visualizer/scrollerFontYScale", getEffect()->getFontScaleY());
    settings.setValue("Visualizer/reflection", getEffect()->getReflectionEnabled());
    settings.setValue("Visualizer/scrollerCustomText", getEffect()->getCustomScrolltextEnabled());
    settings.setValue("Visualizer/scrollerCustomTextContent", getEffect()->getCustomScrolltext());
    settings.setValue("Visualizer/starfield", getEffect()->getStarsEnabled());
    settings.setValue("Visualizer/rasterBars", getEffect()->getRasterBarsEnabled());
    settings.setValue("Visualizer/rotatingObject", getEffect()->getRotatingObjectEnabled());
    settings.setValue("Visualizer/rotatingObjectWireframe", getEffect()->getRotatingObjectWireframeEnabled());
    settings.setValue("Visualizer/rotatingObjectModel", getEffect()->getRotatingObjectModel());
    settings.setValue("Visualizer/rotatingObjectOrbit", getEffect()->getRotatingObjectOrbit());
    settings.setValue("Visualizer/rotatingObjectMaterialColor", getEffect()->getRotatingObjectColor());
    settings.setValue("Visualizer/rotatingObjectModelSize", getEffect()->getRotatingObjectSize());
    settings.setValue("Visualizer/rotatingObjectFocalLength", getEffect()->getRotatingObjectFocalLength());
    settings.setValue("Visualizer/rotatingObjectOrbitSize", getEffect()->getRotatingObjectOrbitSize());
    settings.setValue("Visualizer/rotatingObjectOrbitSpeed", getEffect()->getRotatingObjectOrbitSpeed());
    settings.setValue("Visualizer/rotatingObjectWireframeColor", getEffect()->getRotatingObjectColorWireframe());
    settings.setValue("Visualizer/rotatingObjectMaterial", getEffect()->getRotatingObjectMaterial());
    settings.setValue("Visualizer/rasterBarsAmount", getEffect()->getNumberOfRasterBars());
    settings.setValue("Visualizer/rasterBarsSpeed", getEffect()->getRasterBarsSpeed());
    settings.setValue("Visualizer/rasterBarsHeight", getEffect()->getRasterBarsHeight());
    settings.setValue("Visualizer/rasterBarsVerticalSpacing", getEffect()->getRasterBarsVerticalSpacing());
    settings.setValue("Visualizer/rasterBarsOpacity", getEffect()->getRasterbarsOpacity());
    settings.setValue("Visualizer/starfieldAmount", getEffect()->getNumberOfStars());
    settings.setValue("Visualizer/starfieldDirection", getEffect()->getStarsDirection());
    settings.setValue("Visualizer/starfieldSpeed", getEffect()->getStarSpeed());
    settings.setValue("Visualizer/scroller", getEffect()->getScrollerEnabled());
    settings.setValue("Visualizer/printer", getEffect()->getPrinterEnabled());
    settings.setValue("Visualizer/vuMeter", getEffect()->isVuMeterEnabled());
    settings.setValue("Visualizer/reflectionColor", getEffect()->getReflectionColor());
    settings.setValue("Visualizer/scrollerFontImage", getEffect()->getFont());
    settings.setValue("Visualizer/reflectionOpacity", getEffect()->getReflectionOpacity());
    settings.setValue("Visualizer/scrollerSinusFontScaling", getEffect()->getSinusFontScalingEnabled());
    settings.setValue("Visualizer/maintainAspectRatio", getEffect()->getKeepAspectRatio());
    settings.setValue("Visualizer/resolutionWidth", getEffect()->getResolutionWidth());
    settings.setValue("Visualizer/resolutionHeight", getEffect()->getResolutionHeight());
    settings.setValue("Visualizer/printerFontImage", getEffect()->getPrinterFont());
    settings.setValue("Visualizer/printerFontXScale", getEffect()->getPrinterFontScaleX());
    settings.setValue("Visualizer/printerFontYScale", getEffect()->getPrinterFontScaleY());
    settings.setValue("Internal/columnSamplesNumberWidth", sampleColumnNumberWidth);
    settings.setValue("Internal/columnSamplesNameWidth", sampleColumnNameWidth);
    settings.setValue("Internal/columnSamplesSizeWidth", sampleColumnSizeWidth);
    settings.setValue("Internal/columnSamplesLoopStartWidth", sampleColumnLoopStartWidth);
    settings.setValue("Internal/columnSamplesLoopEndWidth", sampleColumnLoopEndWidth);
    settings.setValue("Internal/columnSamplesVolumeWidth", sampleColumnVolumeWidth);
    settings.setValue("Internal/columnSamplesFineTuneWidth", sampleColumnFinetuneWidth);
    settings.setValue("Internal/columnSamplesResolutionWidth", sampleColumnResolutionWidth);
    settings.setValue("Internal/columnInstrumentsNumberWidth", instrumentColumnNumberWidth);
    settings.setValue("Internal/columnInstrumentsNameWidth", instrumentColumnNameWidth);
    settings.setValue("Internal/columnInstrumentsVolumeWidth", instrumentColumnVolumeWidth);
    settings.setValue("Internal/columnInstrumentsWaveLengthWidth", instrumentColumnWaveLengthWidth);
    settings.setValue("Internal/columnInfoNameWidth", infoNameWidth);
    settings.setValue("Internal/columnInfoValueWidth", infoValueWidth);
    settings.setValue("allowOnlyOneInstance", isOnlyOneInstanceEnabled());
    settings.setValue("Internal/lastOpenedDir", lastDir);

    savePlayListSettings();

    if (PLUGIN_libsidplayfp_LIB != "") {
        settings.setValue(QString("Plugins/libsidplayfpBundledHvscSonglengthsUpdate"),
                          bundledHvscSonglengthsUpdateFrequency);
    }
}

void MainWindow::savePlayListSettings() const {
    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    // clear old playlist settings
    settings.remove("InternalPlaylists");

    // iterate all current playlists and save the order and the column settings
    QStringList orderedKeys;
    settings.beginGroup("InternalPlaylists");

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        orderedKeys << ui->listWidget->item(i)->text();
        const QListWidgetItem *item = ui->listWidget->item(i);
        if (QString key = item->text(); tableWidgetPlaylists.contains(key)) {
            const QTableView *view = tableWidgetPlaylists.value(key);
            settings.setValue(key, view->horizontalHeader()->saveState());
        }
    }

    settings.endGroup();
    settings.setValue("Internal/playlistsOrder", orderedKeys);
}

vector<PlaylistItem *> MainWindow::getPlayListEntriesM3u(QString filename) const {
    vector<PlaylistItem *> entries;
    QFile file(filename);
    //    Song song;
    //    song.startTime = 0;
    //    song.length = 0;
    //    song.title = "";
    //    song.startSubsong = 0;
    //    song.startSubsongPlayList = -1;

    QString extInfoTitle = "";
    QString extInfoLength = "";

    if (!file.open(QIODevice::ReadOnly)) {
        return entries;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString line = "";

    while (!line.isNull()) {
        auto playlistItem = new PlaylistItem();

        line = in.readLine();

        if (line.startsWith("#EXTINF", Qt::CaseInsensitive)) {
            // we got some extrainfo (a title), we save it for the next file
            int i = line.indexOf(',');

            if (i == -1) // invalid format, but we show everything except "#EXTINF"
            {
                i = 7;
            }

            extInfoTitle = line.mid(i + 1).trimmed();
            extInfoLength = line.mid(8, i - 8).trimmed();
            //addDebugText("title from m3u: "+ extInfoTitle);
        } else if (!line.startsWith("#EXTM3U", Qt::CaseInsensitive) && !line.startsWith(
                       "#EXTINF", Qt::CaseInsensitive)) {
            QStringList NEZlist = line.split(NEZPLAYLISTSPLITTER);

            QStringList NEZParameterList;

            if (NEZlist.size() > 1) {
                // has extended NEZ info
                line = NEZlist.at(0);
                NEZParameterList = NEZlist.at(1).split(PLAYLISTFIELDSPLITTER);
            }

            if (QDir fileInPlayList(line);
                fileInPlayList.isAbsolute()) {
                // replace all \ with /
                line = line.replace('\\', '/');
                int i = line.lastIndexOf('/');
                QString onlyFilename = line.mid(i + 1);
                playlistItem->filename = onlyFilename.trimmed();
                playlistItem->fullPath = QString(line).trimmed();
                playlistItem->title = playlistItem->filename;
            } else // it's a relative path
            {
                // url?
                if (line.startsWith("file:///", Qt::CaseInsensitive)) {
                    QUrl url(line);
                    line = url.toLocalFile();
                    //addDebugText("url: " + line);
                    playlistItem->filename = line.trimmed();
                    playlistItem->fullPath = line.trimmed();
                    playlistItem->title = playlistItem->filename;
                } else if (line.startsWith("file://", Qt::CaseInsensitive)) {
                    QUrl url(line);
                    line = url.toLocalFile();
                    addDebugText("url: " + line);
                    playlistItem->filename = line.trimmed();
                    playlistItem->fullPath = line.trimmed();
                    playlistItem->title = playlistItem->filename;
                }
                // if it's a web location, don't add the playlist path
                else if (!line.startsWith("http://", Qt::CaseInsensitive) && !line.startsWith(
                             "https://", Qt::CaseInsensitive)) {
                    // get the path of the playlist file
                    int i = filename.lastIndexOf('/');
                    QString playlistpath = filename.left(i);
                    QString completepath = playlistpath + "/" + line;
                    completepath = completepath.replace('\\', '/');
                    i = completepath.lastIndexOf('/');
                    QString onlyFilename = completepath.mid(i + 1);
                    playlistItem->filename = onlyFilename.trimmed();
                    playlistItem->title = playlistItem->filename;
                    playlistItem->fullPath = QString(completepath).trimmed();
                } else {
                    playlistItem->filename = line.trimmed();
                    playlistItem->fullPath = line.trimmed();
                    playlistItem->title = playlistItem->filename;
                }
            }

            playlistItem->fullPath = QUrl::fromPercentEncoding(playlistItem->fullPath.toStdString().c_str());
            playlistItem->subsongs = 1;

            // check if we have extrainfo
            if (extInfoTitle != "") {
                playlistItem->title = extInfoTitle;
                extInfoTitle = "";
            }

            playlistItem->length = -1;

            if (extInfoLength != "") {
                if (extInfoLength != "-1" && extInfoLength != "0") {
                    playlistItem->length = extInfoLength.toInt() * 1000;
                } else {
                    playlistItem->length = -1;
                }

                extInfoLength = "";
            }

            // check if we have NEZ extrainfo
            if (NEZParameterList.size() > 3) {
                if (!NEZParameterList.at(2).isEmpty()) {
                    playlistItem->title = NEZParameterList.at(2);
                }

                QStringList timelist = NEZParameterList.at(3).split(":");


                float seconds = 0;
                int minutes = 0;
                int hours = 0;

                if (timelist.size() == 3) {
                    hours = timelist.at(0).toInt() * 3600;
                    minutes = timelist.at(1).toInt() * 60;
                    seconds = timelist.at(2).toFloat();
                }

                if (timelist.size() == 2) {
                    minutes = timelist.at(0).toInt() * 60;
                    seconds = timelist.at(1).toFloat();
                }

                if (timelist.size() == 1) {
                    seconds = timelist.at(0).toFloat();
                }

                if (hours + minutes + seconds > 0) {
                    playlistItem->length = (hours + minutes + seconds) * 1000;
                }

                bool subsongOK = false;

                if (NEZParameterList.at(1).startsWith("$")) {
                    QString strStartSubsong = NEZParameterList.at(1).mid(1);
                    playlistItem->startSubsongPlayList = strStartSubsong.toInt(&subsongOK, 16);
                } else {
                    playlistItem->startSubsongPlayList = NEZParameterList.at(1).toInt(&subsongOK);
                }

                if (!subsongOK) {
                    playlistItem->startSubsongPlayList = -1;
                }

                playlistItem->fileFormat = NEZParameterList.at(0);

                if (NEZParameterList.size() > 7) {
                    playlistItem->artist = NEZParameterList.at(7);
                }
            }

            if (!playlistItem->filename.isEmpty()) {
                QFile file(playlistItem->filename);
                QFileInfo fileInfo(file.fileName());
                QString filename(fileInfo.fileName());

                QStringList ignorePrefixList = getIgnorePrefix().split(";");
                QStringListIterator itIgnorePrefixFiles(ignorePrefixList);
                QStringList ignoreSuffixList = getIgnoreSuffix().split(";");
                QStringListIterator itIgnoreSuffixFiles(ignoreSuffixList);
                bool ignoreThisFile = false;

                while (itIgnorePrefixFiles.hasNext()) {
                    if (QString ignoreFilePrefix = itIgnorePrefixFiles.next() + ".";
                        filename.startsWith(ignoreFilePrefix, Qt::CaseInsensitive)) {
                        ignoreThisFile = true;
                    }
                }

                while (itIgnoreSuffixFiles.hasNext()) {
                    if (QString ignoreFileSuffix = "." + itIgnoreSuffixFiles.next();
                        filename.endsWith(ignoreFileSuffix, Qt::CaseInsensitive)) {
                        ignoreThisFile = true;
                    }
                }

                if (!ignoreThisFile) {
                    entries.push_back(playlistItem);
                }
            }
        }
    }

    return entries;
}

QString MainWindow::createPlaylist(const QString &name) {
    const auto newItem = new QListWidgetItem;

    QString newFilename = name;
    int suffix = 0;

    while (tableWidgetPlaylists.contains(newFilename)) {
        suffix++;
        newFilename = name + " (" + QString::number(suffix) + ")" PLAYLIST_DEFAULT_EXTENSION;
    }

    newItem->setText(newFilename);
    newItem->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
    ui->listWidget->insertItem(ui->listWidget->count(), newItem);
    const auto item = new MyItemDelegate(this);
    item->setMainColor(QColor(colorMain.left(7)));
    ui->listWidget->setItemDelegate(item);

    const QFont roboto("Roboto");

    const auto tv = new DraggableTableView();
    tv->setDragBackgroundColor(QColor(colorMain.left(7)));
    tv->setDragTextColor(QColor(colorMainText.left(7)));
    tv->setupDelegate(); // has to be called after colors are set

    const auto pm = new PlaylistModel(this);
    const auto proxyModel = new QSortFilterProxyModel(pm); // create proxy
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

void MainWindow::on_pushButtonNewPlaylist_clicked() {
    createPlaylist("New Playlist" PLAYLIST_DEFAULT_EXTENSION);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item) {
    selectedPlaylist = item->text();
    dockWidgetPlaylist->setWindowTitle(item->text());
    dockWidgetPlaylist->setWidget(tableWidgetPlaylists[item->text()]);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
    // change playlist
    // play first song

    // remove highlighted playlist
    // there might be a state where no playlist is highlighted if a user
    // removed a playing playlist
    if (const QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);
        !l.isEmpty()) {
        l.at(0)->setForeground(QColor(colorMainText.left(7)));
    }

    removeHighlight();
    currentPlaylist = ui->listWidget->currentItem()->text();

    if (isShuffleEnabled()) {
        resetShuffle(currentPlaylist);
    }

    removeHighlight();
    currentRow = 0;

    playSongAtRow(currentRow);
}

bool MainWindow::isShuffleEnabled() const {
    return ui->checkBoxShuffle->checkState() == Qt::Checked;
}

void MainWindow::iffWriteChunkHeader(FILE *f, const char *chunkName, uint32_t chunkLen) {
    fwrite(chunkName, sizeof(int32_t), 1, f);
    chunkLen = SWAP32(chunkLen);
    fwrite(&chunkLen, sizeof(int32_t), 1, f);
}

void MainWindow::iffWriteUint32(FILE *f, uint32_t value) {
    value = SWAP32(value);
    fwrite(&value, sizeof(int32_t), 1, f);
}

void MainWindow::iffWriteUint16(FILE *f, uint16_t value) {
    value = SWAP16(value);
    fwrite(&value, sizeof(int16_t), 1, f);
}

void MainWindow::iffWriteUint8(FILE *f, const uint8_t value) {
    fwrite(&value, sizeof(int8_t), 1, f);
}

void MainWindow::iffWriteChunkData(FILE *f, const void *data, const size_t length) {
    fwrite(data, sizeof(int8_t), length, f);
    if (length & 1) fputc(0, f); // write pad byte if chunk size is uneven
}

void MainWindow::on_checkBoxShuffle_clicked() {
    if (isShuffleEnabled()) {
        resetShuffle(currentPlaylist);
        ui->checkBoxShuffle->setToolTip("Disable shuffle");
    } else {
        ui->checkBoxShuffle->setToolTip("Enable shuffle");
    }

    updateButtons();
}

void MainWindow::on_checkBoxLoop_clicked() {
    if (ui->checkBoxLoop->checkState() == Qt::Unchecked) {
        playMode = normal;
        ui->checkBoxLoop->setToolTip("Enable repeat");
    } else if (ui->checkBoxLoop->checkState() == Qt::PartiallyChecked) {
        playMode = repeatPlaylist;
        ui->checkBoxLoop->setToolTip("Enable repeat one");
    } else {
        ui->checkBoxLoop->setToolTip("Disable repeat");
        playMode = repeatSong;
    }

    updateButtons();
}

void MainWindow::on_checkBoxLoopPoints_clicked() {
    switch (loopPointsState) {
        case UNSET:
            loopPointA = ui->positionSlider->value();
            loopPointsState = A_SET;
            ui->checkBoxLoopPoints->setToolTip("Set ending loop point");
            break;
        case A_SET:
            if (ui->positionSlider->value() > loopPointA) {
                loopPointB = ui->positionSlider->value();
            } else {
                loopPointB = loopPointA;
                loopPointA = ui->positionSlider->value();
            }

            loopPointsState = B_SET;
            ui->checkBoxLoopPoints->setToolTip("Disable loop points");
            break;
        case B_SET:
            loopPointsState = UNSET;
            ui->checkBoxLoopPoints->setToolTip("Set starting loop point");
            break;
        default:
            return;
    }

    updateButtons();
}

void MainWindow::clickSysTrayIcon(const QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        restoreFromTray();
    }
}

void MainWindow::restoreFromTray() {
    // restore from system tray
    if (this->isMinimized()) {
        if (wasMaximized) {
            showMaximized();
        } else {
            showNormal();
        }

        activateWindow();

        this->setWindowTitle(this->isVisible() ? PROJECT_NAME_VERSIONED : windowTitle);
    }

    // minimize to system tray (user clicked on system tray)
    else if (this->isVisible()) {
        wasMaximized = isMaximized();

        hide();

        tray->setToolTip(windowTitle);
    }

    // restore from system tray but when we clicked system tray (and not minimized window)
    else {
        show();
        activateWindow();

        this->setWindowTitle(this->isVisible() ? PROJECT_NAME_VERSIONED : windowTitle);
    }
}

void MainWindow::quit() {
    playStarted = false;

    const auto &sm = SoundManager::getInstance();

    sm.stop();
    sm.shutdown();

    for (auto const &e: tableWidgetPlaylists.keys()) {
        savePlayList(userPath + PLAYLISTS_DIR + "/" + e, userPath + PLAYLISTS_DIR + "/" + e);
    }

    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("Internal/geometry", saveGeometry());
    settings.setValue("Internal/windowState", saveState());
    settings.setValue("Internal/dockingState", dockManager->saveState());
    QApplication::quit();
}

void MainWindow::setColorMain(QString mainColor) {
    mainColor = mainColor + "/*main*/";
    this->colorMainOld = this->colorMain;
    this->colorMain = mainColor;
    changeStyleSheetColor();
}

void MainWindow::setColorMainHover(QString mainColorHover) {
    mainColorHover = mainColorHover + "/*main hover*/";
    this->colorMainHoverOld = this->colorMainHover;
    this->colorMainHover = mainColorHover;
    changeStyleSheetColor();
}

void MainWindow::setColorMedium(QString mediumColor) {
    mediumColor = mediumColor + "/*medium*/";
    this->colorMediumOld = this->colorMedium;
    this->colorMedium = mediumColor;
    changeStyleSheetColor();
}

void MainWindow::setColorBackground(QString backgroundColor) {
    backgroundColor = backgroundColor + "/*background*/";
    this->colorBackgroundOld = this->colorBackground;
    this->colorBackground = backgroundColor;
    changeStyleSheetColor();
}

void MainWindow::setColorBehindBackground(QString behindBackgroundColor) {
    behindBackgroundColor = behindBackgroundColor + "/*behind-background*/";
    this->colorBehindBackgroundOld = this->colorBehindBackground;
    this->colorBehindBackground = behindBackgroundColor;
    changeStyleSheetColor();
}

void MainWindow::setColorMainText(QString mainTextColor) {
    mainTextColor = mainTextColor + "/*main text*/";
    this->colorMainTextOld = this->colorMainText;
    this->colorMainText = mainTextColor;
    changeStyleSheetColor();
}

void MainWindow::setColorSelection(QString selectionColor) {
    selectionColor = selectionColor + "/*selection*/";
    this->colorSelectionOld = this->colorSelection;
    this->colorSelection = selectionColor;
    changeStyleSheetColor();
}

void MainWindow::setColorButton(QString buttonColor) {
    buttonColor = buttonColor + "/*button*/";
    this->colorButtonOld = this->colorButton;
    this->colorButton = buttonColor;
    changeStyleSheetColor();
}

void MainWindow::setColorButtonHover(QString buttonColorHover) {
    buttonColorHover = buttonColorHover + "/*button hover*/";
    this->colorButtonHoverOld = this->colorButtonHover;
    this->colorButtonHover = buttonColorHover;
    changeStyleSheetColor();
}

void MainWindow::setColorDimmedText(QString dimmedTextColor) {
    dimmedTextColor = dimmedTextColor + "/*dimmed text*/";
    this->colorDimmedTextOld = this->colorDimmedText;
    this->colorDimmedText = dimmedTextColor;
    changeStyleSheetColor();
}

void MainWindow::setColorVisualizerTop(const QString &newColor) {
    this->colorVisualizerTop = newColor;
    ui->visualizer->getEffect()->setColorVisualizerTop(QColor(newColor));
}

void MainWindow::setColorVisualizerBottom(const QString &newColor) {
    this->colorVisualizerBottom = newColor;
    ui->visualizer->getEffect()->setColorVisualizerBottom(QColor(newColor));
}

void MainWindow::setColorVisualizerPeakColor(const QString &newColor) {
    this->colorVisualizerPeak = newColor;
    ui->visualizer->getEffect()->setColorVisualizerPeakColor(newColor);
}

void MainWindow::setVuMeterPeaksEnabled(const bool enabled) {
    this->vuMeterPeaksEnabled = enabled;
    ui->visualizer->getEffect()->setVuMeterPeaksEnabled(enabled);
}

void MainWindow::setVuMeterPeaksHeight(const int height) {
    this->vuMeterPeaksHeight = height;
    ui->visualizer->getEffect()->setHeightVisualizerPeak(height);
}

void MainWindow::setColorVisualizerMiddle(const QString &newColor) {
    this->colorVisualizerMiddle = newColor;
    ui->visualizer->getEffect()->setColorVisualizerMiddle(QColor(newColor));
}

void MainWindow::setColorVisualizerBackground(const QString &newColor) {
    this->colorVisualizerBackground = newColor;
    ui->visualizer->getEffect()->setColorVisualizerBackground(QColor(newColor));
}

Effect *MainWindow::getEffect() const {
    return ui->visualizer->getEffect();
}

void MainWindow::setNowPlayingFontSize(const int fontSize) {
    nowPlayingFontSize = fontSize;
    QFont font = ui->labelFilename->font();
    font.setPixelSize(nowPlayingFontSize);
    ui->labelFilename->setFont(font);
}

void MainWindow::setPlaylistRowHeight(const int newRowHeight) {
    playlistRowHeight = newRowHeight;

    QMapIterator i(tableWidgetPlaylists);

    while (i.hasNext()) {
        i.next();
        tableWidgetPlaylists[i.key()]->verticalHeader()->setDefaultSectionSize(playlistRowHeight);
    }
}

void MainWindow::setPlaylistsRowHeight(const int newRowHeight) {
    playlistsRowHeight = newRowHeight;

    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        item->setSizeHint(QSize(playlistsRowHeight, playlistsRowHeight));
    }
}

void MainWindow::createTrayMenu() {
    tray = new QSystemTrayIcon(this);

    const auto trayIcon = QIcon(":/static/data/resources/icon.png");

    tray->setIcon(trayIcon);
    tray->setToolTip(PROJECT_NAME);

    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
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

    tray->setContextMenu(trayIconMenu);
}

void MainWindow::on_checkBoxVolumeOn_clicked() {
    const bool mute = ui->checkBoxVolumeOn->checkState() != Qt::Checked;
    const QString strMute = mute ? "true" : "false";

    SoundManager::getInstance().setMute(mute);
    isVolumeMuted = mute;

    muteAction->setText(isVolumeMuted ? "Unmute" : "Mute");

    addDebugText("Mute volume: " + strMute);
    updateButtons();
}

void MainWindow::muteVolume() {
    isVolumeMuted = !isVolumeMuted;
    ui->checkBoxVolumeOn->setChecked(!isVolumeMuted);

    muteAction->setText("Unmute");

    muteAction->setText(isVolumeMuted ? "Unmute" : "Mute");

    SoundManager::getInstance().setMute(isVolumeMuted);
    updateButtons();
}

void MainWindow::on_volumeSlider_valueChanged(const int value) {
    const int vol = value;
    const auto &sm = SoundManager::getInstance();

    sm.setVolume(static_cast<float>(vol) / 100);
    sm.setMute(false);
    isVolumeMuted = false;
    ui->checkBoxVolumeOn->setCheckState(Qt::Checked);
    ui->checkBoxVolumeOn->setIcon(icons["speaker-on"]);
}

void MainWindow::on_playlist_doubleClicked(const QModelIndex &index) const {
    addDebugText(" dbl clicked");
}

void MainWindow::on_actionRestore_Default_triggered() {
    restoreLayout();
}

void MainWindow::on_actionNew_Workspace_triggered() {
    DialogNewWorkspace dialogNewWorkspace(this);
    dialogNewWorkspace.exec();
}

void MainWindow::on_actionDelete_Workspace_triggered() {
    DialogDeleteWorkspace dialogDeleteWorkspace(this);
    dialogDeleteWorkspace.exec();
}

void MainWindow::on_actionPreferences_triggered() {
    openSettings();
}

void MainWindow::on_actionAdd_folder_triggered() {
    addFolder();
}

void MainWindow::on_actionAdd_file_s_triggered() {
    addFiles();
}

void MainWindow::on_actionAdd_network_stream_triggered() {
    bool ok;
    const QString url = QInputDialog::getText(this, tr("Add URL"),
                                              tr(
                                                  "Enter URL to open:\nFor example: http://www.myfavradio.com:80/stream/1234"),
                                              QLineEdit::Normal, nullptr, &ok);

    if (ok && !url.isEmpty()) {
        QList<QUrl> list;
        list.append(QUrl(url));
        addSong(list, 0, selectedPlaylist, false);
    }
}

void MainWindow::on_actionQuit_triggered() {
    quit();
}

void MainWindow::on_actionAbout_BZR_Player_triggered() {
    about about(this);
    about.exec();
}

void MainWindow::on_pitchSlider_valueChanged(int value) const {
    SoundManager::getInstance().setFrequencyByMultiplier(ui->pitchSlider->value() / 100.0);
    ui->labelPitchValue->setText(QString::number(ui->pitchSlider->value()) + " %");
}

void MainWindow::on_positionSlider_valueChanged(const int value) const {
    const auto timeToShow = QString(msToNiceStringExact(value, displayMilliseconds));
    ui->labelTimer_2->setText(timeToShow);
}

void MainWindow::setFmodSeamlessLoopEnabled(const bool seamlessLoop) {
    isFmodSeamlessLoopEnabled = seamlessLoop;
}

bool MainWindow::getFmodSeamlessLoopEnabled() const {
    return isFmodSeamlessLoopEnabled;
}

const QString &MainWindow::getColorMain() const {
    return colorMain;
}

const QString &MainWindow::getColorMainHover() const {
    return colorMainHover;
}

const QString &MainWindow::getColorMedium() const {
    return colorMedium;
}

const QString &MainWindow::getColorBackground() const {
    return colorBackground;
}

const QString &MainWindow::getColorBehindBackground() const {
    return colorBehindBackground;
}

const QString &MainWindow::getColorMainText() const {
    return colorMainText;
}

const QString &MainWindow::getColorSelection() const {
    return colorSelection;
}

const QString &MainWindow::getColorButton() const {
    return colorButton;
}

const QString &MainWindow::getColorButtonHover() const {
    return colorButtonHover;
}

const QString &MainWindow::getColorDimmedText() const {
    return colorDimmedText;
}

const QString &MainWindow::getColorVisualizerTop() const {
    return colorVisualizerTop;
}

const QString &MainWindow::getColorVisualizerBottom() const {
    return colorVisualizerBottom;
}

const QString &MainWindow::getColorVisualizerMiddle() const {
    return colorVisualizerMiddle;
}

const QString &MainWindow::getColorVisualizerBackground() const {
    return colorVisualizerBackground;
}

const QString &MainWindow::getColorVisualizerPeakColor() const {
    return colorVisualizerPeak;
}

void MainWindow::changeStyleSheetColor() {
    ui->visualizer->setBackgroundColor(colorBackground.left(7));
    ui->trackerView->setBackgroundColor(colorBackground.left(7));
    QString stylesheet;

    QMapIterator i(tableWidgetPlaylists);

    while (i.hasNext()) {
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

    stylesheet = ui->checkBoxLoopPoints->styleSheet();
    stylesheet.replace(colorMainOld, colorMain);
    stylesheet.replace(colorMainHoverOld, colorMainHover);
    ui->checkBoxLoopPoints->setStyleSheet(stylesheet);

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

    stylesheet = ui->checkBoxLoopPoints->styleSheet();
    stylesheet.replace(colorButtonOld, colorButton);
    stylesheet.replace(colorButtonHoverOld, colorButtonHover);
    ui->checkBoxLoopPoints->setStyleSheet(stylesheet);

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

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorBackgroundOld, colorBackground);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorBehindBackgroundOld, colorBehindBackground);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorMediumOld, colorMedium);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorMainTextOld, colorMainText);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = dockManager->styleSheet();
    stylesheet.replace(colorDimmedTextOld, colorDimmedText);
    dockManager->setStyleSheet(stylesheet);

    stylesheet = ui->listWidget->styleSheet();
    stylesheet.replace(colorSelectionOld, colorSelection);
    ui->listWidget->setStyleSheet(stylesheet);

    settingsWindow settingsWindow(this);
    settingsWindow.changeStyleSheetColor();
}

void MainWindow::on_buttonStop_clicked() {
    resetAll();

    auto &sm = SoundManager::getInstance();

    sm.shutdown();

    if (sm.isWavWriterDeviceSelected()) {
        sm.Init(FMOD_OUTPUTTYPE_NOSOUND, ""); // set sound device to silent
    }
}

void MainWindow::resetAll() {
    const auto &sm = SoundManager::getInstance();

    sm.stop();
    //ui->visualizer->stop();
    playAction->setText("Play");
    sm.release();

    if (sm.info != nullptr) {
        sm.info->clear();
    }

    playStarted = false;
    loaded = false;

    // clear all windows/text etc.

    updateSamplesAndInstruments();
    ui->tableInfo->clearContents();
    ui->tableInfo->setRowCount(0);

    getLength();
    ui->positionSlider->setValue(0);

    ui->labelTimer_2->setText(displayMilliseconds ? "0:00.000" : "0:00");

    ui->labelFilename->clear();
    this->setWindowTitle(PROJECT_NAME_VERSIONED);
    channels->updateChannels();

    removeHighlight();
    updateButtons();
}

void MainWindow::removeHighlight() {
    if (tableWidgetPlaylists.contains(currentPlaylist)) {
        const QModelIndex index3 = tableWidgetPlaylists[currentPlaylist]->model()->index(currentRow, 7, QModelIndex());
        tableWidgetPlaylists[currentPlaylist]->model()->setData(index3, false, Qt::EditRole);
        tableWidgetPlaylists[currentPlaylist]->update();
    }
}

bool MainWindow::isVuMeterPeaksEnabled() const {
    return vuMeterPeaksEnabled;
}

int MainWindow::isVuMeterPeaksHeight() const {
    return vuMeterPeaksHeight;
}

bool MainWindow::isOnlyOneInstanceEnabled() const {
    return allowOnlyOneInstance;
}

void MainWindow::setAllowOnlyOneInstanceEnabled(const bool enabled) {
    allowOnlyOneInstance = enabled;
}

void MainWindow::setDisplayMilliseconds(const bool enabled) {
    displayMilliseconds = enabled;
}

void MainWindow::setEnqueueItems(const bool enabled) {
    enqueueItems = enabled;
}

QString MainWindow::getVersion() {
    return VERSION;
}

void MainWindow::sendSocketMsg() const {
    QStringList args = qApp->arguments();
    args.removeFirst(); // remove first argument from commandline since it's always "BZRPlayer.exe"
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

void MainWindow::acceptConnection() {
    tcpClient = tcpServer->nextPendingConnection();
    connect(tcpClient, SIGNAL(readyRead()), SLOT(getSocketData()));
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError) {
    //if (socketError == QTcpSocket::RemoteHostClosedError)
    //    return;

    QMessageBox::information(this, "Network error", "The following error occurred: " + tcpClient->errorString());
    tcpClient->close();
    tcpServer->close();
}

/*
 * Reads all the data from the socket
 * the data read is what the instance got from the command line
 * if m_bEnqueueFileEnabled is true then we add all sounds to the playlist
 * if it is false, we play them directly (only the last one)
 */
void MainWindow::getSocketData() {
    const QByteArray bytes = tcpClient->readAll();

    QDataStream dataStreamRead(bytes);

    QStringList list;
    dataStreamRead >> list;
    //QMessageBox::information(this, "list","count: " + QString::number(list.count()));

    QList<QUrl> urls;

    for (const auto &item: list) {
        urls.append(QUrl().fromLocalFile(item));
    }

    if (list.count() <= 0) {
        return;
    }

    const QString &command = list.last();

    if (command.isEmpty()) {
        return;
    }

    if (command.toLower() == "-play") {
        //on_buttonPlay_clicked(false,true);
    } else if (command.toLower() == "-pause") {
        //on_buttonPause_clicked();
    } else if (command.toLower() == "-next") {
        //on_buttonNext_clicked();
    } else if (command.toLower() == "-prev") {
        //on_buttonPrev_clicked();
    } else if (command.toLower() == "-stop") {
        //on_buttonStop_clicked();
    } else if (command.toLower() == "-nextsubsound") {
        //on_buttonNextSub_clicked();
    } else if (command.toLower() == "-prevsubsound") {
        //on_buttonPrevSub_clicked();
    } else if (command.toLower() == "-clear") {
        //m_playListWindow->clear(true);
    } else if (command.toLower() == "-quit") {
        //close();
    } else if (command.toLower().startsWith("-volume[") && command.endsWith("]")) {
        //            bool ok = false;
        //            int volume = QString(command.mid(8,command.length()-9)).toInt(&ok);
        //            if(ok)
        //            {
        //                ui.sliderVolume->setValue(volume);
        //            }
    } else // no command, just add/play the file
    {
        //            if(m_bEnqueueFileEnabled)
        //            {
        //                m_playListWindow->addSong(QStringList(QStringList(bytes).last()),-1);
        //            }
        //            else
        //{
        //                m_playListWindow->clear(true);
        //                m_iCurrentSong=0;
        //                m_playListWindow->addSong(QStringList(QStringList(bytes).last()),-1);
        //                // play first
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

        if ((allowOnlyOneInstance && !enqueueItems) || !allowOnlyOneInstance) {
            const QList<QListWidgetItem *> l = ui->listWidget->findItems(currentPlaylist, Qt::MatchExactly);

            l.at(0)->setForeground(QColor(colorMainText.left(7)));
            currentPlaylist = PLAYLIST_DEFAULT_FILENAME;

            on_listWidget_itemClicked(ui->listWidget->item(0));
            removeHighlight();

            currentRow = rowCountBeforeAddSong;

            playSongAtRow(currentRow);
        }
        //}
    }
}

QPixmap MainWindow::changeSvgColor(const QString &svgPath, const QColor &color, const QRectF *region,
                                   const QColor &color2) {
    QSvgRenderer renderer(svgPath);
    const QSize size = renderer.defaultSize();
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);

    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    if (region) {
        if (color2 != nullptr) {
            painter.fillRect(pixmap.rect(), color2);
        }

        const QRectF regionPx(region->x() * size.width(),
                              region->y() * size.height(),
                              region->width() * size.width(),
                              region->height() * size.height());
        painter.setClipRect(regionPx);
    }

    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return pixmap;
}

void MainWindow::loadWorkspaces() {
    QAction *restore = ui->menuRestore_Layout->actions().first();
    workspaceSeparator = ui->menuRestore_Layout->insertSeparator(restore);

    const QDir directory(userPath + LAYOUTS_DIR);
    QStringList workspaces = directory.entryList(QStringList() << "*.ini", QDir::Files);

    foreach(QString filename, workspaces) {
        QFileInfo fileInfo(filename);
        QString basename = fileInfo.baseName();
        const auto action = new QAction(basename);

        ui->menuRestore_Layout->insertAction(workspaceSeparator, action);
        connect(action, &QAction::triggered, this, [&, this, filename] { slot_LoadWorkspace(filename); });
    }
}

void MainWindow::deleteWorkspace(const QString &workspace) const {
    foreach(QAction *action, ui->menuRestore_Layout->actions()) {
        if (action->isSeparator()) {
        } else if (action->menu()) {
        } else {
            if (action->text() == workspace) {
                ui->menuRestore_Layout->removeAction(action);
            }
        }
    }
}

void MainWindow::slot_LoadWorkspace(const QString &filename) {
    if (const QSettings settings(userPath + LAYOUTS_DIR + "/" + filename, QSettings::IniFormat);
        !dockManager->restoreState(settings.value("Internal/dockingState").toByteArray())) {
        QMessageBox::critical(this, "Error", "Couldn't load layout");
    }
}

void MainWindow::createNewWorkspace(const QString &filename) {
    QString fileNameAndExtension = filename + ".ini";
    QSettings settings(userPath + LAYOUTS_DIR + "/" + fileNameAndExtension, QSettings::IniFormat);

    settings.setValue("Internal/dockingState", dockManager->saveState());

    const auto action = new QAction(filename);

    ui->menuRestore_Layout->insertAction(workspaceSeparator, action);

    connect(action, &QAction::triggered, this, [&, this, fileNameAndExtension] {
        slot_LoadWorkspace(fileNameAndExtension);
    });
}

void MainWindow::downloadHvscSonglengthsComplete() {
    if (filedownloader->downloadedData().isEmpty()) {
        addDebugText("Failed to download " + filedownloader->getUrl().toString());
        return;
    }

    const QString hvscSonglengthsDownloadPath = userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH;

    QFile file(hvscSonglengthsDownloadPath);

    if (!file.open(QIODevice::ReadWrite)) {
        addDebugText("Couldn't write to file " + file.fileName());
        return;
    }

    QTextStream stream(&file);
    stream << filedownloader->downloadedData();
    file.close();

    bundledHvscSonglengthsDownloadEpoch = QDateTime::currentSecsSinceEpoch();
    bundledHvscSonglengthsPath = hvscSonglengthsDownloadPath;

    const settingsWindow settingsWindow(this);

    if (hvscSonglengthsPath.compare(dataPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH) == 0) {
        hvscSonglengthsPath = hvscSonglengthsDownloadPath;

        settingsWindow.setUiLineEditLibsidplayfpHvscSonglengthsPath(hvscSonglengthsDownloadPath);
        settingsWindow.saveSettingsLibsidplayfp();
    }

    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("Plugins/libsidplayfpBundledHvscSonglengthsPath", hvscSonglengthsDownloadPath);
    settings.setValue("Plugins/libsidplayfpBundledHvscSonglengthsDownloadEpoch",
                      bundledHvscSonglengthsDownloadEpoch);
}

void MainWindow::setBundledHvscSonglengthsUpdateFrequency(const QString &freq) {
    bundledHvscSonglengthsUpdateFrequency = freq;
}

// swaps columns so that artist column is first for default and new playlists
void MainWindow::swapColumns(QTableView *tableview) {
    tableview->setColumnWidth(0, 200);
    tableview->setColumnWidth(1, 100);
    tableview->setColumnWidth(2, 55);
    tableview->setColumnWidth(3, 75);
    tableview->setColumnWidth(8, 175);

    tableview->horizontalHeader()->swapSections(1, 8);
}

void MainWindow::setupIcons() {
    QPixmap shuffleOn = changeSvgColor(":/resources/shuffle.svg", colorMain.left(7));
    QPixmap shuffleOnHover = changeSvgColor(":/resources/shuffle.svg", colorMainHover.left(7));
    QPixmap shuffleOff = changeSvgColor(":/resources/shuffle.svg", colorButton.left(7));
    QPixmap shuffleOffHover = changeSvgColor(":/resources/shuffle.svg", colorButtonHover.left(7));

    QPixmap speakerOn = changeSvgColor(":/resources/speaker.svg", colorButton.left(7));
    QPixmap speakerOnHover = changeSvgColor(":/resources/speaker.svg", colorButtonHover.left(7));
    QPixmap speakerOff = changeSvgColor(":/resources/speaker-off.svg", colorButton.left(7));
    QPixmap speakerOffHover = changeSvgColor(":/resources/speaker-off.svg", colorButtonHover.left(7));

    QPixmap play = changeSvgColor(":/resources/play.svg", colorButton.left(7));
    QPixmap playHover = changeSvgColor(":/resources/play.svg", colorButtonHover.left(7));
    QPixmap pause = changeSvgColor(":/resources/pause.svg", colorButton.left(7));
    QPixmap pauseHover = changeSvgColor(":/resources/pause.svg", colorButtonHover.left(7));
    QPixmap pauseDisabled = changeSvgColor(":/resources/pause.svg", colorMain.left(7));

    QPixmap stop = changeSvgColor(":/resources/stop.svg", colorButton.left(7));
    QPixmap stopHover = changeSvgColor(":/resources/stop.svg", colorButtonHover.left(7));

    QPixmap prev = changeSvgColor(":/resources/prev.svg", colorButton.left(7));
    QPixmap prevHover = changeSvgColor(":/resources/prev.svg", colorButtonHover.left(7));
    QPixmap next = changeSvgColor(":/resources/next.svg", colorButton.left(7));
    QPixmap nextHover = changeSvgColor(":/resources/next.svg", colorButtonHover.left(7));

    QPixmap repeat = changeSvgColor(":/resources/repeat.svg", colorButton.left(7));
    QPixmap repeatHover = changeSvgColor(":/resources/repeat.svg", colorButtonHover.left(7));
    QPixmap repeat1 = changeSvgColor(":/resources/repeat-1.svg", colorMain.left(7));
    QPixmap repeat1Hover = changeSvgColor(":/resources/repeat-1.svg", colorMainHover.left(7));
    QPixmap repeatOn = changeSvgColor(":/resources/repeat.svg", colorMain.left(7));
    QPixmap repeatOnHover = changeSvgColor(":/resources/repeat.svg", colorMainHover.left(7));

    QPixmap loopPointsOffDisabled = changeSvgColor(":/resources/loop-points.svg", colorMedium.left(7));
    QPixmap loopPointsOff = changeSvgColor(":/resources/loop-points.svg", colorButton.left(7));
    QPixmap loopPointsOffHover = changeSvgColor(":/resources/loop-points.svg", colorButtonHover.left(7));
    QPixmap loopPointAOn = changeSvgColor(":/resources/loop-points.svg", colorMain.left(7),
                                          new QRectF(0.35, 0, 1, 1), colorButton.left(7));
    QPixmap loopPointsOn = changeSvgColor(":/resources/loop-points.svg", colorMain.left(7));
    QPixmap loopPointAOnHover = changeSvgColor(":/resources/loop-points.svg", colorMainHover.left(7),
                                               new QRectF(0.35, 0, 1, 1), colorButtonHover.left(7));
    QPixmap loopPointsOnHover = changeSvgColor(":/resources/loop-points.svg", colorMainHover.left(7));

    QPixmap add = changeSvgColor(":/resources/add.svg", colorButton.left(7));
    QPixmap addHover = changeSvgColor(":/resources/add.svg", colorButtonHover.left(7));

    QPixmap checkBoxOn = changeSvgColor(":/resources/checkbox-on.svg", colorMain.left(7));
    QPixmap checkBoxOff = changeSvgColor(":/resources/checkbox-off.svg", colorBehindBackground.left(7));
    QPixmap checkBoxOnDisabled = changeSvgColor(":/resources/checkbox-on-disabled.svg", colorMedium.left(7));
    QPixmap checkBoxOffDisabled = changeSvgColor(":/resources/checkbox-off-disabled.svg", colorMedium.left(7));

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
    icons["pause-disabled"] = pauseDisabled;
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
    icons["loop-points-off-disabled"] = loopPointsOffDisabled;
    icons["loop-points-off"] = loopPointsOff;
    icons["loop-points-off-hover"] = loopPointsOffHover;
    icons["loop-point-a-on"] = loopPointAOn;
    icons["loop-points-on"] = loopPointsOn;
    icons["loop-point-a-on-hover"] = loopPointAOnHover;
    icons["loop-points-on-hover"] = loopPointsOnHover;
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

void MainWindow::setupAdvancedDockingSystem() {
    ads::CDockManager::setConfigFlags(ads::CDockManager::DefaultOpaqueConfig);
    ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaHasTabsMenuButton, false);
    ads::CDockManager::setConfigFlag(ads::CDockManager::RetainTabSizeWhenCloseButtonHidden, true);

    dockManager = new ads::CDockManager(this);

    dockManager->setStyleSheet(
        "QScrollBar:vertical {background-color: #282828/*background*/;     width: 15px;     margin: 0 3px 0 3px;     border: 1px transparent #282828/*background*/;     border-radius: 4px; } QScrollBar::handle:vertical {     background-color: #404040/*medium*/;     min-height: 25px;     border-radius: 4px; } QScrollBar:horizontal {     background-color: #282828/*background*/;     height: 15px;     margin: 3px 0 3px 0;     border: 1px transparent #282828/*background*/;     border-radius: 4px; } QScrollBar::handle:horizontal {     background-color: #404040/*medium*/;     min-width: 5px;     border-radius: 4px; } QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {     background: none; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {     background: none; } QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {     background: none; } QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {     background: none; } QScrollBar::sub-line:vertical {     height: 0;     width: 0; } QScrollBar::add-line:vertical {     height: 0;     width: 0; } QScrollBar::sub-line:horizontal {     height: 0;     width: 0; } QScrollBar::add-line:horizontal {     height: 0;     width: 0; }QScrollArea#dockWidgetScrollArea {	background: #282828/*background*/;padding: 0px;	border: none;} ads--CDockWidgetTab QLabel {font-family:Roboto;font-size:14px;color:#b1b1b1/*dimmed text*/;}ads--CDockWidgetTab[activeTab=\"true\"] QLabel {color:#ffffff/*main text*/;}#tabCloseButton {margin-top: 0;background: none;border: none;}ads--CDockContainerWidget ads--CDockSplitter::handle {background: #161616/*behind-background*/;}ads--CDockAreaTitleBar { background: #282828/*background*/;}ads--CDockWidgetTab {background: #282828/*background*/}");

    dockWidgetLogMessages = new ads::CDockWidget(dockManager, "Log Messages");
    dockWidgetLogMessages->setWidget(ui->dockWidgetContents_9, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetLogMessages);
    dockWidgets.append(dockWidgetLogMessages);

    dockWidgetTitle = new ads::CDockWidget(dockManager, "Now Playing");
    dockWidgetTitle->setWidget(ui->dockWidgetContents_5, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetTitle);
    dockWidgets.append(dockWidgetTitle);

    dockWidgetChannels = new ads::CDockWidget(dockManager, "Channels");
    dockWidgetChannels->setWidget(ui->dockWidgetContents_7);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetChannels);
    dockWidgets.append(dockWidgetChannels);

    dockWidgetTrackerView = new ads::CDockWidget(dockManager, "Tracker View");
    dockWidgetTrackerView->setWidget(ui->dockWidgetContents_6, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetTrackerView);
    dockWidgets.append(dockWidgetTrackerView);

    dockWidgetInfo = new ads::CDockWidget(dockManager, "Info");
    dockWidgetInfo->setWidget(ui->dockWidgetContents_3, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetInfo);
    dockWidgets.append(dockWidgetInfo);

    dockWidgetVisualizer = new ads::CDockWidget(dockManager, "Visualizer");
    dockWidgetVisualizer->setWidget(ui->dockWidgetContents_11, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetVisualizer);
    dockWidgets.append(dockWidgetVisualizer);

    dockWidgetPlaylists = new ads::CDockWidget(dockManager, "Playlists");
    dockWidgetPlaylists->setWidget(ui->dockWidgetContents_8, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::LeftDockWidgetArea, dockWidgetPlaylists);
    dockWidgets.append(dockWidgetPlaylists);

    dockWidgetPlaylist = new ads::CDockWidget(dockManager, "Playlist");
    dockWidgetPlaylist->setWidget(ui->dockWidgetContents_4, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::LeftDockWidgetArea, dockWidgetPlaylist);
    dockWidgets.append(dockWidgetPlaylist);

    dockWidgetInstruments = new ads::CDockWidget(dockManager, "Instruments");
    dockWidgetInstruments->setWidget(ui->dockWidgetContents_10, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::RightDockWidgetArea, dockWidgetInstruments);
    dockWidgets.append(dockWidgetInstruments);

    dockWidgetSamples = new ads::CDockWidget(dockManager, "Samples");
    dockWidgetSamples->setWidget(ui->dockWidgetContents_2, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::RightDockWidgetArea, dockWidgetSamples);
    dockWidgets.append(dockWidgetSamples);

    dockWidgetPlayControl = new ads::CDockWidget(dockManager, "Play Control");
    dockWidgetPlayControl->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
    dockWidgetPlayControl->setWidget(ui->dockWidgetContents, ads::CDockWidget::ForceNoScrollArea);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, dockWidgetPlayControl);
    dockWidgets.append(dockWidgetPlayControl);
}

QStringList MainWindow::sortPreservingOrder(const QStringList &folderPlaylists,
                                            const QStringList &sortedPlaylistOrder) {
    QStringList result;
    QSet<QString> added;

    for (const QString &key: sortedPlaylistOrder) {
        if (folderPlaylists.contains(key)) {
            result.append(key);
            added.insert(key);
        }
    }

    for (const QString &item: folderPlaylists) {
        if (!added.contains(item)) {
            result.append(item);
        }
    }

    return result;
}
