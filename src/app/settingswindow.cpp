#include "settingswindow.h"
#include "album.h"
#include "qmessagebox.h"
#include "qsettings.h"
#include "ui_settingswindow.h"
#include <QColorDialog>
#include <QFileDialog>
#include <cmath>
#include <fstream>
#include <QDebug>
#include <QFontDatabase>
#include "plugins.h"

settingsWindow::settingsWindow(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::settingsWindow)
{
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);
    mainWindow = static_cast<MainWindow*>(this->parent());


    //    QRect geo;
    //    geo = ui->groupBoxLibOpenMPT->geometry();
    //    geo.setLeft(170);
    //    ui->groupBoxLibOpenMPT->setGeometry(geo);

    //    geo = ui->groupBoxLibsid->geometry();
    //    geo.setLeft(170);
    //    ui->groupBoxLibsid->setGeometry(geo);

    //    geo = ui->groupBoxUADE->geometry();
    //    geo.setLeft(170);
    //    ui->groupBoxUADE->setGeometry(geo);

    //    geo = ui->scrollArea->geometry();
    //    geo.setLeft(170);
    //    ui->scrollArea->setGeometry(geo);

    //    geo = ui->scrollAreaAppearance->geometry();
    //    geo.setLeft(170);
    //    ui->scrollAreaAppearance->setGeometry(geo);

    //    geo = ui->scrollAreaVisualizer->geometry();
    //    geo.setLeft(170);
    //    ui->scrollAreaVisualizer->setGeometry(geo);

    //    geo = ui->tableWidgetPlugins->geometry();
    //    geo.setLeft(170);
    //    ui->tableWidgetPlugins->setGeometry(geo);


    ui->sliderVUMeterWidth->installEventFilter(this);
    ui->sliderVumeterOpacity->installEventFilter(this);
    ui->sliderAmplitude->installEventFilter(this);
    ui->sliderFrequency->installEventFilter(this);
    ui->sliderSinusSpeed->installEventFilter(this);
    ui->sliderScrollSpeed->installEventFilter(this);
    ui->sliderVerticalScrollPosition->installEventFilter(this);
    ui->sliderScrollerXScale->installEventFilter(this);
    ui->sliderScrollerYScale->installEventFilter(this);
    ui->sliderPrinterXScale->installEventFilter(this);
    ui->sliderPrinterYScale->installEventFilter(this);
    ui->sliderPeakHeight->installEventFilter(this);
    ui->sliderReflectionOpacity->installEventFilter(this);
    ui->sliderPlaylistsRowHeight->installEventFilter(this);
    ui->sliderRowHeight->installEventFilter(this);
    ui->sliderNowPlayingFontSize->installEventFilter(this);
    ui->sliderResetVolumeToValue->installEventFilter(this);
    ui->SliderNormalizerFadeTim->installEventFilter(this);
    ui->SliderNormalizerMaxAmp->installEventFilter(this);
    ui->SliderNormalizerThreshold->installEventFilter(this);
    ui->sliderNumberOfStars->installEventFilter(this);
    ui->sliderStarSpeed->installEventFilter(this);
    ui->sliderResolutionWidth->installEventFilter(this);
    ui->sliderResolutionHeight->installEventFilter(this);
    ui->sliderRasterBarsBarHeight->installEventFilter(this);
    ui->sliderRasterBarsSpeed->installEventFilter(this);
    ui->sliderRasterbarsOpacity->installEventFilter(this);
    ui->sliderRasterBarsVerticalSpacing->installEventFilter(this);
    ui->sliderNumberOfRasterBars->installEventFilter(this);


    ui->comboBoxFilterOpenMPT->installEventFilter(this);
    ui->comboBoxFilterOpenMPT->addItem("Auto", "auto");
    ui->comboBoxFilterOpenMPT->addItem("A500 Filter", "a500");
    ui->comboBoxFilterOpenMPT->addItem("A1200 Filter", "a1200");
    ui->comboBoxFilterOpenMPT->addItem("Unfiltered", "unfiltered");

    ui->comboBoxResamplingOpenMPT->installEventFilter(this);
    ui->comboBoxResamplingOpenMPT->addItem("Internal Default", "0");
    ui->comboBoxResamplingOpenMPT->addItem("No Interpolation (1 tap)", "1");
    ui->comboBoxResamplingOpenMPT->addItem("Linear (2 taps)", "2");
    ui->comboBoxResamplingOpenMPT->addItem("Cubic (4 taps)", "4");
    ui->comboBoxResamplingOpenMPT->addItem("Sinc (8 taps)", "8");

    ui->comboBoxDitherOpenMPT->installEventFilter(this);
    ui->comboBoxDitherOpenMPT->addItem("No dithering", "0");
    ui->comboBoxDitherOpenMPT->addItem("Default", "1");
    ui->comboBoxDitherOpenMPT->addItem("Rectangular, 0.5 bit depth", "2");
    ui->comboBoxDitherOpenMPT->addItem("Rectangular, 1 bit depth", "3");

    ui->SliderStereoSeparationOpenMPT->installEventFilter(this);


    ui->sliderSilenceTimeOut->installEventFilter(this);

    ui->comboBoxReverb->installEventFilter(this);
    ui->comboBox->installEventFilter(this);
    ui->comboBoxFilter->installEventFilter(this);
    ui->comboBoxFilter->addItem("Auto", "Auto");
    ui->comboBoxFilter->addItem("Always On", "Always On");
    ui->comboBoxFilter->addItem("Always Off", "Always Off");

    ui->comboBox->addItem("Default", FMOD_OUTPUTTYPE_AUTODETECT);
    ui->comboBox->addItem("No Sound", FMOD_OUTPUTTYPE_NOSOUND);
    ui->comboBox->addItem("WAV Writer", FMOD_OUTPUTTYPE_WAVWRITER);
    ui->comboBox->addItem("Windows Audio Session API", FMOD_OUTPUTTYPE_WASAPI);
    ui->comboBox->addItem("ASIO 2.0", FMOD_OUTPUTTYPE_ASIO);
    ui->comboBox->addItem("Windows Sonic", FMOD_OUTPUTTYPE_WINSONIC);

    int index = ui->comboBox->findData(mainWindow->getOutputDevice());
    ui->comboBox->setCurrentIndex(index);

    ui->comboBoxReverb->addItem("Generic", "Generic");
    ui->comboBoxReverb->addItem("Padded cell", "Padded cell");
    ui->comboBoxReverb->addItem("Room", "Room");
    ui->comboBoxReverb->addItem("Bathroom", "Bathroom");
    ui->comboBoxReverb->addItem("Living room", "Living room");
    ui->comboBoxReverb->addItem("Stone room", "Stone room");
    ui->comboBoxReverb->addItem("Auditorium", "Auditorium");
    ui->comboBoxReverb->addItem("Concert hall", "Concert hall");
    ui->comboBoxReverb->addItem("Cave", "Cave");
    ui->comboBoxReverb->addItem("Arena", "Arena");
    ui->comboBoxReverb->addItem("Hangar", "Hangar");
    ui->comboBoxReverb->addItem("Carpeted hallway", "Carpeted hallway");
    ui->comboBoxReverb->addItem("Hallway", "Hallway");
    ui->comboBoxReverb->addItem("Stone corridor", "Stone corridor");
    ui->comboBoxReverb->addItem("Alley", "Alley");
    ui->comboBoxReverb->addItem("Forest", "Forest");
    ui->comboBoxReverb->addItem("City", "City");
    ui->comboBoxReverb->addItem("Mountains", "Mountains");
    ui->comboBoxReverb->addItem("Quarry", "Quarry");
    ui->comboBoxReverb->addItem("Plain", "Plain");
    ui->comboBoxReverb->addItem("Parking lot", "Parking lot");
    ui->comboBoxReverb->addItem("Sewer pipe", "Sewer pipe");
    ui->comboBoxReverb->addItem("Underwater", "Underwater");

    mainWindow->addDebugText("settings window getReverbPreset: " + mainWindow->getReverbPreset());
    index = ui->comboBoxReverb->findData(mainWindow->getReverbPreset());

    ui->comboBoxReverb->setCurrentIndex(index);

    ui->checkBoxOnlyOneInstance->setChecked(mainWindow->isOnlyOneInstanceEnabled());
    ui->checkBoxEnqueueItems->setChecked(mainWindow->getEnqueueItems());
    ui->checkBoxEnqueueItems->setEnabled(mainWindow->isOnlyOneInstanceEnabled());
    ui->labelEnqueueItems->setEnabled(mainWindow->isOnlyOneInstanceEnabled());

    ui->checkBoxResetVolume->setChecked(mainWindow->getResetVolume());
    ui->sliderResetVolumeToValue->setValue(mainWindow->getResetVolumeValue());

    ui->checkBoxNormalizer->setChecked(mainWindow->getNormalizeEnabled());
    mainWindow->
        addDebugText("mainWindow->getNormalizeEnabled(): " + QString::number(mainWindow->getNormalizeEnabled()));

    ui->checkBoxReverb->setChecked(mainWindow->getReverbEnabled());
    mainWindow->addDebugText("mainWindow->getReverbEnabled(): " + QString::number(mainWindow->getReverbEnabled()));


    ui->checkBoxMilliseconds->setChecked(mainWindow->getDisplayMilliseconds());


    ui->checkBoxSystrayOnQuit->setChecked(mainWindow->getSystrayOnQuitEnabled());

    ui->SliderNormalizerFadeTim->setValue(mainWindow->getNormalizeFadeTime());
    ui->SliderNormalizerThreshold->setValue(mainWindow->getNormalizeThreshold());
    ui->SliderNormalizerMaxAmp->setValue(mainWindow->getNormalizeMaxAmp());

    ui->lineEditIgnoreSuffix->setText(mainWindow->getIgnoreSuffix());
    ui->lineEditIgnorePrefix->setText(mainWindow->getIgnorePrefix());

    ui->sliderRowHeight->setValue(mainWindow->getPlaylistRowHeight());
    ui->sliderPlaylistsRowHeight->setValue(mainWindow->getPlaylistsRowHeight());
    ui->sliderNowPlayingFontSize->setValue(mainWindow->getNowPlayingFontSize());

    ui->sliderVUMeterWidth->setValue(mainWindow->getEffect()->getVumeterWidth());
    ui->sliderVumeterOpacity->setValue(mainWindow->getEffect()->getVumeterOpacity());
    ui->sliderPeakHeight->setValue((mainWindow->getVUMeterPeaksHeight()));
    ui->checkBoxPeaks->setChecked((mainWindow->getVUMeterPeaksEnabled()));
    ui->sliderAmplitude->setValue(mainWindow->getEffect()->getAmplitude());
    ui->sliderFrequency->setValue(mainWindow->getEffect()->getFrequency() * 10000);
    ui->sliderSinusSpeed->setValue(mainWindow->getEffect()->getSinusSpeed() * 100);
    ui->sliderScrollSpeed->setValue(mainWindow->getEffect()->getScrollSpeed());
    ui->sliderVerticalScrollPosition->setValue(mainWindow->getEffect()->getVerticalScrollPosition());
    ui->sliderScrollerXScale->setValue(mainWindow->getEffect()->getFontScaleX());
    ui->sliderScrollerYScale->setValue(mainWindow->getEffect()->getFontScaleY());
    ui->sliderPrinterXScale->setValue(mainWindow->getEffect()->getPrinterFontScaleX());
    ui->sliderPrinterYScale->setValue(mainWindow->getEffect()->getPrinterFontScaleY());
    ui->checkBoxReflectionEnabled->setChecked(mainWindow->getEffect()->getReflectionEnabled());
    ui->sliderReflectionOpacity->setValue(mainWindow->getEffect()->getReflectionOpacity());
    ui->sliderNumberOfStars->setValue(mainWindow->getEffect()->getNumberOfStars());
    ui->sliderStarSpeed->setValue(mainWindow->getEffect()->getStarSpeed());
    ui->checkBoxStarsEnabled->setChecked(mainWindow->getEffect()->getStarsEnabled());
    ui->checkBoxScrollerEnabled->setChecked(mainWindow->getEffect()->getScrollerEnabled());
    ui->checkBoxCustomScrolltextEnabled->setChecked(mainWindow->getEffect()->getCustomScrolltextEnabled());
    ui->textEditCustomScrolltext->setText(mainWindow->getEffect()->getCustomScrolltext());
    ui->checkBoxPrinterEnabled->setChecked(mainWindow->getEffect()->getPrinterEnabled());
    ui->checkBoxVUMeterEnabled->setChecked(mainWindow->getEffect()->getVUMeterEnabled());
    ui->checkBoxSinusFontScaling->setChecked(mainWindow->getEffect()->getSinusFontScalingEnabled());
    ui->comboBoxStarsDirection->installEventFilter(this);
    ui->comboBoxStarsDirection->addItem("Left", "left");
    ui->comboBoxStarsDirection->addItem("Right", "right");
    ui->comboBoxStarsDirection->addItem("Up", "up");
    ui->comboBoxStarsDirection->addItem("Down", "down");
    ui->comboBoxStarsDirection->addItem("In", "in");
    ui->comboBoxStarsDirection->addItem("Out", "out");
    int dirindex = ui->comboBoxStarsDirection->findData(mainWindow->getEffect()->getStarsDirection());
    ui->comboBoxStarsDirection->setCurrentIndex(dirindex);
    ui->sliderResolutionWidth->setValue(mainWindow->getEffect()->getResolutionWidth());
    ui->sliderResolutionHeight->setValue(mainWindow->getEffect()->getResolutionHeight());
    ui->checkBoxAspectRatio->setChecked(mainWindow->getEffect()->getKeepAspectRatio());

    ui->checkBoxRasterbarsEnabled->setChecked(mainWindow->getEffect()->getRasterBarsEnabled());
    ui->sliderNumberOfRasterBars->setValue(mainWindow->getEffect()->getNumberOfRasterBars());
    ui->sliderRasterBarsBarHeight->setValue(mainWindow->getEffect()->getRasterBarsHeight());
    ui->sliderRasterBarsVerticalSpacing->setValue(mainWindow->getEffect()->getRasterBarsVerticalSpacing());
    ui->sliderRasterBarsSpeed->setValue(mainWindow->getEffect()->getRasterBarsSpeed());
    ui->sliderRasterbarsOpacity->setValue(mainWindow->getEffect()->getRasterbarsOpacity());
    forceUpdateToSliders();


    ui->comboBoxSidSongFileUpdateFrequency->installEventFilter(this);
    ui->comboBoxSidSongFileUpdateFrequency->addItem("At every start", "At every start");
    ui->comboBoxSidSongFileUpdateFrequency->addItem("Daily", "Daily");
    ui->comboBoxSidSongFileUpdateFrequency->addItem("Weekly", "Weekly");
    ui->comboBoxSidSongFileUpdateFrequency->addItem("Monthly", "Monthly");
    ui->comboBoxSidSongFileUpdateFrequency->addItem("Never", "Never");

    index = ui->comboBoxSidSongFileUpdateFrequency->findData(mainWindow->getSIDSongLengthFrequency());
    ui->comboBoxSidSongFileUpdateFrequency->setCurrentIndex(index);


    int extensionPos = mainWindow->getEffect()->getFont().lastIndexOf('.');
    QString thumb(mainWindow->getEffect()->getFont().left(extensionPos) + ".thumb.png");
    ui->buttonScrollerFontImage->setIcon(QIcon(thumb));
    extensionPos = mainWindow->getEffect()->getPrinterFont().lastIndexOf('.');
    QString thumbP(mainWindow->getEffect()->getPrinterFont().left(extensionPos) + ".thumb.png");
    ui->buttonPrinterFontImage->setIcon(QIcon(thumbP));


    loadSidplaySettings();
    loadUADESettings();
    loadlibopenmptSettings();


    updateColorButtons();


    QFont roboto("Roboto");
    QStringList columns;
    columns << tr("TITLE") << tr("VERSION");

    ui->tableWidgetPlugins->setShowGrid(false);
    ui->tableWidgetPlugins->setFrameShape(QFrame::NoFrame);
    ui->tableWidgetPlugins->setFrameShadow(QFrame::Plain);
    ui->tableWidgetPlugins->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPlugins->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetPlugins->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableWidgetPlugins->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableWidgetPlugins->setFont(roboto);
    ui->tableWidgetPlugins->setFocusPolicy(Qt::NoFocus);
    ui->tableWidgetPlugins->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPlugins->horizontalHeader()->setSortIndicatorShown(false);
    ui->tableWidgetPlugins->verticalHeader()->setVisible(false);
    ui->tableWidgetPlugins->setWordWrap(false);

    ui->tableWidgetPlugins->setHorizontalHeaderLabels(columns);
    ui->tableWidgetPlugins->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidgetPlugins->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ui->tableWidgetPlugins->setColumnWidth(0, 135);
    ui->tableWidgetPlugins->setColumnWidth(1, 65);

    ui->tableWidgetPlugins->setRowCount(PLUGINS_COUNT);
    int row = 0;

    ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_fmod_VERSION));
    ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_fmod_NAME));

    if (PLUGIN_adplug_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_adplug_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_adplug_NAME));
    }
    if (PLUGIN_asap_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_asap_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_asap_NAME));
    }
    if (PLUGIN_audiodecoder_wsr_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_audiodecoder_wsr_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_audiodecoder_wsr_NAME));
    }
    if (PLUGIN_audiofile_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_audiofile_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_audiofile_NAME));
    }
    if (PLUGIN_faad2_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_faad2_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_faad2_NAME));
    }
    if (PLUGIN_flod_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_flod_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_flod_NAME));
    }
    if (PLUGIN_furnace_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_furnace_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_furnace_NAME));
    }
    if (PLUGIN_game_music_emu_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_game_music_emu_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_game_music_emu_NAME));
    }
    if (PLUGIN_highly_experimental_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_experimental_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_experimental_NAME));
    }
    if (PLUGIN_highly_quixotic_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_quixotic_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_quixotic_NAME));
    }
    if (PLUGIN_highly_theoretical_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_theoretical_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_theoretical_NAME));
    }
    if (PLUGIN_hivelytracker_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_hivelytracker_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_hivelytracker_NAME));
    }
    if (PLUGIN_jaytrax_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_jaytrax_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_jaytrax_NAME));
    }
    if (PLUGIN_kdm_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_kdm_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_kdm_NAME));
    }
    if (PLUGIN_klystron_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_klystron_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_klystron_NAME));
    }
    if (PLUGIN_lazyusf2_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_lazyusf2_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_lazyusf2_NAME));
    }
    if (PLUGIN_libfc14audiodecoder_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libfc14audiodecoder_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libfc14audiodecoder_NAME));
    }
    if (PLUGIN_libopenmpt_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libopenmpt_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libopenmpt_NAME));
    }
    if (PLUGIN_libpac_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libpac_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libpac_NAME));
    }
    if (PLUGIN_libsidplayfp_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libsidplayfp_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libsidplayfp_NAME));
    }
    if (PLUGIN_libstsound_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libstsound_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libstsound_NAME));
    }
    if (PLUGIN_libxmp_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libxmp_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libxmp_NAME));
    }
    if (PLUGIN_mdxmini_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_mdxmini_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_mdxmini_NAME));
    }
    if (PLUGIN_organya_decoder_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_organya_decoder_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_organya_decoder_NAME));
    }
    if (PLUGIN_protrekkr_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_protrekkr_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_protrekkr_NAME));
    }
    if (PLUGIN_sc68_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sc68_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sc68_NAME));
    }
    if (PLUGIN_sndh_player_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sndh_player_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sndh_player_NAME));
    }
    if (PLUGIN_sunvox_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sunvox_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sunvox_NAME));
    }
    if (PLUGIN_tunes98_plug_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_tunes98_plug_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_tunes98_plug_NAME));
    }
    if (PLUGIN_v2m_player_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_v2m_player_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_v2m_player_NAME));
    }
    if (PLUGIN_vgmplay_legacy_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_vgmplay_legacy_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_vgmplay_legacy_NAME));
    }
    if (PLUGIN_vgmstream_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_vgmstream_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_vgmstream_NAME));
    }
    if (PLUGIN_vio2sf_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_vio2sf_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_vio2sf_NAME));
    }
    if (PLUGIN_wothke_uade_2_13_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_wothke_uade_2_13_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_wothke_uade_2_13_NAME));
    }
    if (PLUGIN_zxtune_DLL != "")
    {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_zxtune_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_zxtune_NAME));
    }

    QDateTime qdt = QDateTime::fromSecsSinceEpoch(mainWindow->getSIDSongLengthDownloaded());
    if (mainWindow->getSIDSongLengthDownloaded() > 0)
    {
        ui->labelSIDSongLengthDownloaded->setText(
            "Downloaded official Songlengths.md5 to " + mainWindow->getSIDSongPathDownloaded() + " at " + qdt.toString(
                "yyyy-MM-dd hh:mm:ss"));
    }
    else
    {
        ui->labelSIDSongLengthDownloaded->setText("Never downloaded official Songlengths.md5");
    }

    ui->tableWidgetPlugins->sortItems(0);
    ui->tableWidgetPlugins->setRowCount(row);
    ui->fontWidget->setVisible(false);
    ui->fontWidgetPrinter->setVisible(false);
    QDir directory(QApplication::applicationDirPath() + "/data/resources/visualizer/bitmapfonts");
    QStringList images = directory.entryList(QStringList() << "*.thumb.png", QDir::Files);
    foreach(QString filename, images)
    {
        QString fullfilename = QApplication::applicationDirPath() + "/data/resources/visualizer/bitmapfonts/" +
            filename;
        Album* album = new Album(filename);
        album->artwork = fullfilename;
        album->title = filename.replace(".thumb.png", "");
        album->path = fullfilename.replace(".thumb.png", ".png");
        album->putPixmap(album->artwork);
        ui->fontWidget->AddAlbum(album);

        Album* albumPrinter = new Album(filename);
        albumPrinter->artwork = fullfilename;
        albumPrinter->title = filename.replace(".thumb.png", "");
        albumPrinter->path = fullfilename.replace(".thumb.png", ".png");
        albumPrinter->putPixmap(album->artwork);
        ui->fontWidgetPrinter->AddAlbum(albumPrinter);

        connect(album, SIGNAL(clickedAddAlbum(QString)), this, SLOT(loadBitmapFont(QString)));
        connect(albumPrinter, SIGNAL(clickedAddAlbum(QString)), this, SLOT(loadBitmapFontPrinter(QString)));
    }
    updateCheckBoxes();
    on_buttonGeneral_clicked();
    changeStyleSheetColor();
}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::on_sliderResetVolumeToValue_sliderMoved(int position)
{
}

void settingsWindow::on_checkBoxResetVolume_toggled(bool checked)
{
    ui->labelSliderResetVolume->setEnabled(checked);
    ui->sliderResetVolumeToValue->setEnabled(checked);
    if (ui->checkBoxResetVolume->isChecked())
    {
        ui->checkBoxResetVolume->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxResetVolume->setIcon(mainWindow->icons["checkbox-off"]);
    }
}

void settingsWindow::on_sliderResetVolumeToValue_valueChanged(int value)
{
    ui->labelSliderResetVolume->setText(QString::number(value) + " %");
}

void settingsWindow::on_SliderNormalizerFadeTim_valueChanged(int value)
{
    ui->labelNormalizerFadeTimeValue->setText(QString::number(value));
    mainWindow->setNormalizeFadeTime(ui->SliderNormalizerFadeTim->value());
}

void settingsWindow::on_SliderNormalizerThreshold_valueChanged(int value)
{
    ui->labelNormalizerThresholdValue->setText(QString::number(value) + " %");
    mainWindow->setNormalizeThreshold(ui->SliderNormalizerThreshold->value());
}

void settingsWindow::on_SliderNormalizerMaxAmp_valueChanged(int value)
{
    ui->labelNormalizerMaxAmpValue->setText(QString::number(value));
    mainWindow->setNormalizeMaxAmp(ui->SliderNormalizerMaxAmp->value());
}

void settingsWindow::on_checkBoxNormalizer_toggled(bool checked)
{
    mainWindow->addDebugText("on_checkBoxNormalizer_toggled");
    ui->SliderNormalizerFadeTim->setEnabled(checked);
    ui->SliderNormalizerThreshold->setEnabled(checked);
    ui->SliderNormalizerMaxAmp->setEnabled(checked);
    ui->labelNormalizerFadeTimeValue->setEnabled(checked);
    ui->labelNormalizerThresholdValue->setEnabled(checked);
    ui->labelNormalizerMaxAmpValue->setEnabled(checked);

    mainWindow->setNormalizeEnabled(checked);
    if (ui->checkBoxNormalizer->isChecked())
    {
        ui->checkBoxNormalizer->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxNormalizer->setIcon(mainWindow->icons["checkbox-off"]);
    }
}

void settingsWindow::on_checkBoxReverb_toggled(bool checked)
{
    mainWindow->addDebugText("on_checkBoxReverb_toggled");
    ui->labelReverbPreset->setEnabled(checked);
    ui->comboBoxReverb->setEnabled(checked);

    mainWindow->setReverbEnabled(checked);
    if (ui->checkBoxReverb->isChecked())
    {
        ui->checkBoxReverb->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxReverb->setIcon(mainWindow->icons["checkbox-off"]);
    }
}

bool settingsWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel &&
        (
            obj == ui->sliderVUMeterWidth ||
            obj == ui->sliderVumeterOpacity ||
            obj == ui->sliderAmplitude ||
            obj == ui->sliderFrequency ||
            obj == ui->sliderSinusSpeed ||
            obj == ui->sliderScrollSpeed ||
            obj == ui->sliderVerticalScrollPosition ||
            obj == ui->sliderScrollerXScale ||
            obj == ui->sliderScrollerYScale ||
            obj == ui->sliderPrinterXScale ||
            obj == ui->sliderPrinterYScale ||
            obj == ui->sliderPeakHeight ||
            obj == ui->sliderReflectionOpacity ||
            obj == ui->sliderNumberOfStars ||
            obj == ui->sliderStarSpeed ||
            obj == ui->sliderPlaylistsRowHeight ||
            obj == ui->sliderRowHeight ||
            obj == ui->sliderNowPlayingFontSize ||
            obj == ui->sliderResetVolumeToValue ||
            obj == ui->SliderNormalizerFadeTim ||
            obj == ui->SliderNormalizerMaxAmp ||
            obj == ui->SliderNormalizerThreshold ||
            obj == ui->comboBoxReverb ||
            obj == ui->comboBoxFilter ||
            obj == ui->comboBox ||
            obj == ui->comboBoxStarsDirection ||
            obj == ui->sliderResolutionHeight ||
            obj == ui->sliderResolutionWidth ||
            obj == ui->sliderRasterBarsBarHeight ||
            obj == ui->sliderRasterBarsSpeed ||
            obj == ui->sliderRasterbarsOpacity ||
            obj == ui->sliderRasterBarsVerticalSpacing ||
            obj == ui->sliderNumberOfRasterBars ||
            obj == ui->comboBoxFilterOpenMPT ||
            obj == ui->comboBoxResamplingOpenMPT ||
            obj == ui->comboBoxDitherOpenMPT ||
            obj == ui->SliderStereoSeparationOpenMPT ||
            obj == ui->comboBoxSidSongFileUpdateFrequency ||
            obj == ui->sliderSilenceTimeOut
        )
    )

    {
        return true;
    }


    return QWidget::eventFilter(obj, event);
}


void settingsWindow::on_buttonOK_clicked()
{
    if (ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt() != mainWindow->getOutputDevice())
    {
        mainWindow->setOutputDeviceSetting(ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt());
    }
    bool checkedResetVolume = ui->checkBoxResetVolume->checkState() == Qt::Checked ? true : false;
    bool checkedSystrayOnQuit = ui->checkBoxSystrayOnQuit->checkState() == Qt::Checked ? true : false;
    mainWindow->setResetVolume(checkedResetVolume);
    mainWindow->setResetVolumeValue(ui->sliderResetVolumeToValue->value());
    mainWindow->setSystrayOnQuitEnabled(checkedSystrayOnQuit);
    mainWindow->setIgnoreSuffix(ui->lineEditIgnoreSuffix->text());
    mainWindow->setIgnorePrefix(ui->lineEditIgnorePrefix->text());
    mainWindow->getEffect()->setCustomScrolltext(ui->textEditCustomScrolltext->toPlainText());

    saveSidplaySettings();
    saveUADESettings();
    savelibopenmptSettings();
    mainWindow->SaveSettings();
    if (mainWindow->getEffect()->getCustomScrolltextEnabled())
    {
        mainWindow->getEffect()->setScrollText(mainWindow->getEffect()->getCustomScrolltext());
    }
    close();
}


void settingsWindow::on_comboBoxReverb_textActivated(const QString& arg1)
{
    mainWindow->setReverbPreset(arg1);
    bool checkedReverb = ui->checkBoxReverb->checkState() == Qt::Checked ? true : false;
    mainWindow->setReverbEnabled(checkedReverb);
}


void settingsWindow::on_buttonBrowseSonglengths_clicked()
{
    QString startFolder = ui->lineEditSonglength->text();
    if (startFolder.startsWith("/"))
    {
        startFolder = QApplication::applicationDirPath() + startFolder;
    }
    QString file = QFileDialog::getOpenFileName(this, "Choose your Songlengths.md5", startFolder, "*.md5");
    if (!file.isEmpty())
    {
        ui->lineEditSonglength->setText(file);
    }
}

void settingsWindow::loadlibopenmptSettings()
{
    //read config from disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/libopenmpt.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }
    //defaults
    ui->SliderStereoSeparationOpenMPT->setValue(100);
    ui->checkBoxFilterOpenMPT->setChecked(true);
    int index = ui->comboBoxFilterOpenMPT->findData("auto");
    ui->comboBoxFilterOpenMPT->setCurrentIndex(index);
    int index2 = ui->comboBoxResamplingOpenMPT->findData("0");
    ui->comboBoxResamplingOpenMPT->setCurrentIndex(index2);
    int index3 = ui->comboBoxDitherOpenMPT->findData("1");
    ui->comboBoxDitherOpenMPT->setCurrentIndex(index3);
    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("stereo_separation") == 0)
                {
                    ui->SliderStereoSeparationOpenMPT->setValue(atoi(value.c_str()));
                }
                else if (word.compare("emulate_amiga_filter") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        ui->checkBoxFilterOpenMPT->setChecked(true);
                    }
                    else
                    {
                        ui->checkBoxFilterOpenMPT->setChecked(false);
                    }
                }
                else if (word.compare("interpolation_filter") == 0)
                {
                    int index = ui->comboBoxResamplingOpenMPT->findData(value.c_str());
                    ui->comboBoxResamplingOpenMPT->setCurrentIndex(index);
                }
                else if (word.compare("amiga_filter") == 0)
                {
                    int index = ui->comboBoxFilterOpenMPT->findData(value.c_str());
                    ui->comboBoxFilterOpenMPT->setCurrentIndex(index);
                }
                else if (word.compare("dither") == 0)
                {
                    int index = ui->comboBoxDitherOpenMPT->findData(value.c_str());
                    ui->comboBoxDitherOpenMPT->setCurrentIndex(index);
                }
            }
        }
        ifs.close();
    }
}

void settingsWindow::loadUADESettings()
{
    //read config from disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/uade.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;

    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    ui->comboBoxFilter->setCurrentIndex(0);
    ui->checkBoxFilterEnabled->setChecked(false);
    ui->sliderSilenceTimeOut->setValue(5);
    ui->lineEditUADESonglength->setText("/uade.md5");
    ui->checkBoxSongLengthUADE->setChecked(true);

    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("led_forced") == 0)
                {
                    if (value.compare("auto") == 0)
                    {
                        ui->comboBoxFilter->setCurrentIndex(0);
                    }
                    else if (value.compare("on") == 0)
                    {
                        ui->comboBoxFilter->setCurrentIndex(1);
                    }
                    else
                    {
                        ui->comboBoxFilter->setCurrentIndex(2);
                    }
                }
                else if (word.compare("no_filter") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        ui->checkBoxFilterEnabled->setChecked(false);
                    }
                    else
                    {
                        ui->checkBoxFilterEnabled->setChecked(true);
                    }
                }
                else if (word.compare("silence_timeout_enabled") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        ui->checkBoxSilenceTimeout->setChecked(true);
                    }
                    else
                    {
                        ui->checkBoxSilenceTimeout->setChecked(false);
                    }
                }
                else if (word.compare("songlengths_enabled") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        ui->checkBoxSongLengthUADE->setChecked(true);
                    }
                    else
                    {
                        ui->checkBoxSongLengthUADE->setChecked(false);
                    }
                }

                else if (word.compare("silence_timeout") == 0)
                {
                    ui->sliderSilenceTimeOut->setValue(atoi(value.c_str()));
                }
                else if (word.compare("songlengths_path") == 0)
                {
                    if (value == "")
                    {
                        ui->lineEditUADESonglength->setText("/uade.md5");
                    }
                    else
                    {
                        ui->lineEditUADESonglength->setText(value.c_str());
                    }
                }
            }
        }
        ifs.close();
    }
}

void settingsWindow::loadSidplaySettings()
{
    //read config from disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/libsidplayfp.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }
    //Hack, because "toggled" means changing value,
    //so just to be sure, set it to both values so
    //that on_checkBoxSIDSonglengthsEnabled_toggled is called
    ui->checkBoxSIDSonglengthsEnabled->setChecked(false);
    ui->checkBoxSIDSonglengthsEnabled->setChecked(true);
    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("songlengths_path") == 0)
                {
                    if (value == "")
                    {
                        ui->lineEditSonglength->setText("/user/plugin/sid/Songlengths.md5");
                    }
                    else
                    {
                        ui->lineEditSonglength->setText(value.c_str());
                    }
                }
                else if (word.compare("songlengths_path_old") == 0)
                {
                    if (value == "")
                    {
                        ui->lineEditSonglengthOld->setText("/user/plugin/sid/Songlengths.txt");
                    }
                    else
                    {
                        ui->lineEditSonglengthOld->setText(value.c_str());
                    }
                }
                else if (word.compare("songlengths_enabled") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        ui->checkBoxSIDSonglengthsEnabled->setChecked(true);
                    }
                    else
                    {
                        ui->checkBoxSIDSonglengthsEnabled->setChecked(false);
                    }
                }
            }
        }
        ifs.close();
    }
    else
    {
        ui->checkBoxSIDSonglengthsEnabled->setChecked(false);
        ui->checkBoxSIDSonglengthsEnabled->setChecked(true);
        ui->lineEditSonglength->setText("/user/plugin/sid/Songlengths.md5");
        ui->lineEditSonglengthOld->setText("/user/plugin/sid/Songlengths.txt");
    }
}

void settingsWindow::saveSidplaySettings()
{
    //save config to disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/libsidplayfp.cfg";
    ofstream ofs(filename.c_str());
    string line;

    QString songlengthsEnabled;
    if (ui->checkBoxSIDSonglengthsEnabled->isChecked())
    {
        songlengthsEnabled = "true";
    }
    else
    {
        songlengthsEnabled = "false";
    }

    if (ofs.fail())
    {
        //The file could not be opened
        return;
    }

    ofs << "songlengths_enabled=" << songlengthsEnabled.toStdString().c_str() << "\n";
    ofs << "songlengths_path=" << ui->lineEditSonglength->text().toStdString().c_str() << "\n";
    ofs << "songlengths_path_old=" << ui->lineEditSonglengthOld->text().toStdString().c_str() << "\n";
    ofs.close();
}

void settingsWindow::savelibopenmptSettings()
{
    QString filterOpenMPT;
    if (ui->checkBoxFilterOpenMPT->isChecked())
    {
        filterOpenMPT = "true";
    }
    else
    {
        filterOpenMPT = "false";
    }
    //save config to disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/libopenmpt.cfg";
    ofstream ofs(filename.c_str());
    string line;

    if (ofs.fail())
    {
        //The file could not be opened
        return;
    }
    ofs << "stereo_separation=" << ui->SliderStereoSeparationOpenMPT->value() << "\n";
    ofs << "interpolation_filter=" << ui->comboBoxResamplingOpenMPT->currentData().toString().toStdString().c_str() <<
        "\n";
    ofs << "amiga_filter=" << ui->comboBoxFilterOpenMPT->currentData().toString().toStdString().c_str() << "\n";
    ofs << "dither=" << ui->comboBoxDitherOpenMPT->currentData().toString().toStdString().c_str() << "\n";
    ofs << "emulate_amiga_filter=" << filterOpenMPT.toStdString().c_str() << "\n";
    ofs.close();
}

void settingsWindow::saveUADESettings()
{
    //save config to disk
    string filename = QApplication::applicationDirPath().toStdString() + "/user/plugin/config/uade.cfg";
    ofstream ofs(filename.c_str());
    string line;

    if (ofs.fail())
    {
        //The file could not be opened
        return;
    }


    QString filter;

    if (ui->comboBoxFilter->currentIndex() == 0)
    {
        filter = "auto";
    }
    else if (ui->comboBoxFilter->currentIndex() == 1)
    {
        filter = "on";
    }
    else
    {
        filter = "off";
    }

    QString filterEmulated;
    if (ui->checkBoxFilterEnabled->isChecked())
    {
        filterEmulated = "false";
    }
    else
    {
        filterEmulated = "true";
    }
    QString silenceTimeoutEnabled;
    if (ui->checkBoxSilenceTimeout->isChecked())
    {
        silenceTimeoutEnabled = "true";
    }
    else
    {
        silenceTimeoutEnabled = "false";
    }
    QString songlengthsEnabled;
    if (ui->checkBoxSongLengthUADE->isChecked())
    {
        songlengthsEnabled = "true";
    }
    else
    {
        songlengthsEnabled = "false";
    }
    ofs << "songlengths_path=" << ui->lineEditUADESonglength->text().toStdString().c_str() << "\n";
    ofs << "led_forced=" << filter.toStdString().c_str() << "\n";
    ofs << "no_filter=" << filterEmulated.toStdString().c_str() << "\n";
    ofs << "silence_timeout=" << ui->sliderSilenceTimeOut->value() << "\n";
    ofs << "silence_timeout_enabled=" << silenceTimeoutEnabled.toStdString().c_str() << "\n";
    ofs << "songlengths_enabled=" << songlengthsEnabled.toStdString().c_str() << "\n";
    ofs.close();
}

void settingsWindow::on_tableWidgetPlugins_itemClicked(QTableWidgetItem* item)
{
    int row = item->row();

    if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libsidplayfp_NAME)
    {
        ui->groupBoxLibsid->setHidden(false);
        ui->groupBoxUADE->setHidden(true);
        ui->groupBoxLibOpenMPT->setHidden(true);
    }
    else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_wothke_uade_2_13_NAME)
    {
        ui->groupBoxLibsid->setHidden(true);
        ui->groupBoxLibOpenMPT->setHidden(true);
        ui->groupBoxUADE->setHidden(false);
    }
    else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libopenmpt_NAME)
    {
        ui->groupBoxLibsid->setHidden(true);
        ui->groupBoxLibOpenMPT->setHidden(false);
        ui->groupBoxUADE->setHidden(true);
    }
    else
    {
        ui->groupBoxLibOpenMPT->setHidden(true);
        ui->groupBoxLibsid->setHidden(true);
        ui->groupBoxUADE->setHidden(true);
    }
}


void settingsWindow::on_buttonBrowseSonglengthsOld_clicked()
{
    QString startFolder = ui->lineEditSonglengthOld->text();
    if (startFolder.startsWith("/"))
    {
        startFolder = QApplication::applicationDirPath() + startFolder;
    }
    QString file = QFileDialog::getOpenFileName(this, "Choose your Songlengths.txt", startFolder, "*.txt");
    if (!file.isEmpty())
    {
        ui->lineEditSonglengthOld->setText(file);
    }
}

void settingsWindow::changeStyleSheetColor()
{
    QString stylesheet;


    stylesheet = this->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorDimmedTextOld, mainWindow->getColorDimmedText());
    this->setStyleSheet(stylesheet);


    stylesheet = ui->buttonGeneral->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonGeneral->setStyleSheet(stylesheet);

    stylesheet = ui->buttonAppearance->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonAppearance->setStyleSheet(stylesheet);

    stylesheet = ui->buttonVisualizer->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonVisualizer->setStyleSheet(stylesheet);

    stylesheet = ui->buttonPlugins->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonPlugins->setStyleSheet(stylesheet);

    stylesheet = ui->buttonOK->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonOK->setStyleSheet(stylesheet);

    stylesheet = ui->buttonColorsDefault->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonColorsDefault->setStyleSheet(stylesheet);


    stylesheet = ui->scrollAreaWidgetContents->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContents->setStyleSheet(stylesheet);

    stylesheet = ui->scrollAreaWidgetContents_2->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContents_2->setStyleSheet(stylesheet);

    stylesheet = ui->scrollAreaWidgetContents_3->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContents_3->setStyleSheet(stylesheet);

    stylesheet = ui->tableWidgetPlugins->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->tableWidgetPlugins->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxUADE->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxUADE->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibsid->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibsid->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibOpenMPT->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibOpenMPT->setStyleSheet(stylesheet);
}


void settingsWindow::on_buttonMainColor_clicked()
{
    QColor oldColor(mainWindow->getColorMain().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorMain(newColor.name());
        mainWindow->channels->updateChannelColors();

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonMainColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonMediumColor_clicked()
{
    QColor oldColor(mainWindow->getColorMedium().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorMedium(newColor.name());
        mainWindow->channels->updateChannelColors();

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonMediumColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonBackgroundColor_clicked()
{
    QColor oldColor(mainWindow->getColorBackground().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorBackground(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonBackgroundColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonBehindBackgroundColor_clicked()
{
    QColor oldColor(mainWindow->getColorBehindBackground().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorBehindBackground(newColor.name());
        mainWindow->channels->updateChannelColors();

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonBehindBackgroundColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonMainTextColor_clicked()
{
    QColor oldColor(mainWindow->getColorMainText().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorMainText(newColor.name());
        mainWindow->channels->updateChannelColors();


        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonMainTextColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonButtonColor_clicked()
{
    QColor oldColor(mainWindow->getColorButton().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorButton(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonButtonColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonDimmedTextColor_clicked()
{
    QColor oldColor(mainWindow->getColorDimmedText().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorDimmedText(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonDimmedTextColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonMainHoverColor_clicked()
{
    QColor oldColor(mainWindow->getColorMainHover().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorMainHover(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonMainHoverColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonButtonHoverColor_clicked()
{
    QColor oldColor(mainWindow->getColorButtonHover().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorButtonHover(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonButtonHoverColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonColorsDefault_clicked()
{
    mainWindow->resetToDefaultColors();
    updateColorButtons();
}


void settingsWindow::on_buttonVisualizer_clicked()
{
    ui->scrollArea->setHidden(true);
    ui->scrollAreaAppearance->setHidden(true);
    ui->tableWidgetPlugins->setHidden(true);
    ui->groupBoxLibsid->setHidden(true);
    ui->groupBoxLibOpenMPT->setHidden(true);
    ui->groupBoxUADE->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(false);
}


void settingsWindow::on_buttonGeneral_clicked()
{
    ui->scrollArea->setHidden(false);
    ui->scrollAreaAppearance->setHidden(true);
    ui->tableWidgetPlugins->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(true);
    ui->groupBoxUADE->setHidden(true);
    ui->groupBoxLibsid->setHidden(true);
    ui->groupBoxLibOpenMPT->setHidden(true);
}


void settingsWindow::on_buttonPlugins_clicked()
{
    ui->scrollArea->setHidden(true);
    ui->scrollAreaAppearance->setHidden(true);
    ui->tableWidgetPlugins->setHidden(false);
    ui->scrollAreaVisualizer->setHidden(true);
}

void settingsWindow::on_buttonAppearance_clicked()
{
    ui->scrollArea->setHidden(true);
    ui->scrollAreaAppearance->setHidden(false);
    ui->tableWidgetPlugins->setHidden(true);
    ui->groupBoxUADE->setHidden(true);
    ui->groupBoxLibsid->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(true);
    ui->groupBoxLibOpenMPT->setHidden(true);
}

void settingsWindow::on_buttonColorVUMeterTop_clicked()
{
    QColor oldColor(mainWindow->getColorVisualizerTop());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorVisualizerTop(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonColorVUMeterTop->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonColorVUMeterBottom_clicked()
{
    QColor oldColor(mainWindow->getColorVisualizerBottom());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorVisualizerBottom(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonColorVUMeterBottom->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonColorVUMeterMiddle_clicked()
{
    QColor oldColor(mainWindow->getColorVisualizerMiddle());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorVisualizerMiddle(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonColorVUMeterMiddle->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonColorVisualizerBackground_clicked()
{
    QColor oldColor(mainWindow->getColorVisualizerBackground());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorVisualizerBackground(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonColorVisualizerBackground->setStyleSheet(qss);
    }
}

void settingsWindow::on_sliderRowHeight_valueChanged(int value)
{
    ui->labelRowHeight->setText(QString::number(value) + " pixels");
    mainWindow->setPlaylistRowHeight(value);
}


void settingsWindow::on_sliderPlaylistsRowHeight_valueChanged(int value)
{
    ui->labelPlaylistsRowHeight->setText(QString::number(value) + " pixels");
    mainWindow->setPlaylistsRowHeight(value);
}

void settingsWindow::updateColorButtons()
{
    QString qss;
    qss = QString("background-color: %1").arg(mainWindow->getColorMain());
    ui->buttonMainColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorBackground());
    ui->buttonBackgroundColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorBehindBackground());
    ui->buttonBehindBackgroundColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMainText());
    ui->buttonMainTextColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorSelection());
    ui->buttonSelectionColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorDimmedText());
    ui->buttonDimmedTextColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMedium());
    ui->buttonMediumColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorButton());
    ui->buttonButtonColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMainHover());
    ui->buttonMainHoverColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorButtonHover());
    ui->buttonButtonHoverColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerTop());
    ui->buttonColorVUMeterTop->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerBottom());
    ui->buttonColorVUMeterBottom->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerMiddle());
    ui->buttonColorVUMeterMiddle->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerPeakColor());
    ui->buttonColorVUMeterPeak->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getEffect()->getReflectionColor());
    ui->buttonColorReflection->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerBackground());
    ui->buttonColorVisualizerBackground->setStyleSheet(qss);
}

void settingsWindow::on_sliderAmplitude_valueChanged(int value)
{
    mainWindow->getEffect()->setAmplitude(value);
    ui->labelAmplitude->setText(QString::number(value));
}


void settingsWindow::on_sliderFrequency_valueChanged(int value)
{
    mainWindow->getEffect()->setSinusFrequency(value / 10000.0f);
    ui->labelFrequency->setText(QString::number(value / 10000.0f));
}


void settingsWindow::on_sliderSinusSpeed_valueChanged(int value)
{
    mainWindow->getEffect()->setSinusSpeed(value / 100.0f);
    ui->labelSinusSpeed->setText(QString::number(value / 100.0f));
}


void settingsWindow::on_sliderScrollSpeed_valueChanged(int value)
{
    mainWindow->getEffect()->setScrollSpeed(value);
    ui->labelScrollSpeed->setText(QString::number(value));
}


void settingsWindow::on_sliderScrollerXScale_valueChanged(int value)
{
    mainWindow->getEffect()->setFontScaleX(value);
    ui->labelScrollerXScale->setText(QString::number(value));
}


void settingsWindow::on_sliderScrollerYScale_valueChanged(int value)
{
    mainWindow->getEffect()->setFontScaleY(value);
    ui->labelScrollerYScale->setText(QString::number(value));
}


void settingsWindow::on_buttonColorVUMeterPeak_clicked()
{
    QColor oldColor(mainWindow->getColorVisualizerPeakColor());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorVisualizerPeakColor(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonColorVUMeterPeak->setStyleSheet(qss);
    }
}


void settingsWindow::on_checkBoxPeaks_toggled(bool checked)
{
    if (ui->checkBoxPeaks->isChecked())
    {
        ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-off"]);
    }
    mainWindow->setVUMeterPeaksEnabled(checked);
    ui->labelVUMeterPeakColor->setEnabled(checked);
    ui->buttonColorVUMeterPeak->setEnabled(checked);
    ui->labelTitleVUMeterPeak->setEnabled(checked);
    ui->sliderPeakHeight->setEnabled(checked);
    ui->labelVUMeterPeakHeight->setEnabled(checked);
}


void settingsWindow::on_sliderPeakHeight_valueChanged(int value)
{
    mainWindow->setVUMeterPeaksHeight(value);
    ui->labelVUMeterPeakHeight->setText(QString::number(value));
}


void settingsWindow::on_sliderVUMeterWidth_valueChanged(int value)
{
    mainWindow->getEffect()->setVumeterWidth(value);
    ui->labelVUMeterWidth->setText(QString::number(value) + " %");
}


void settingsWindow::on_sliderVerticalScrollPosition_valueChanged(int value)
{
    mainWindow->getEffect()->setVerticalScrollPosition(value);
    ui->labelVerticalScrollPosition->setText(QString::number(value));
}


void settingsWindow::on_checkBoxReflectionEnabled_toggled(bool checked)
{
    bool checkedReflection = ui->checkBoxReflectionEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setReflectionEnabled(checkedReflection);
    if (checkedReflection)
    {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelTitleReflectionColor->setEnabled(true);
        ui->buttonColorReflection->setEnabled(true);
        ui->labelTitleReflectionOpacity->setEnabled(true);
        ui->sliderReflectionOpacity->setEnabled(true);
        ui->labelReflectionOpacity->setEnabled(true);
    }
    else
    {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelTitleReflectionColor->setEnabled(false);
        ui->buttonColorReflection->setEnabled(false);
        ui->labelTitleReflectionOpacity->setEnabled(false);
        ui->sliderReflectionOpacity->setEnabled(false);
        ui->labelReflectionOpacity->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxStarsEnabled_toggled(bool checked)
{
    bool checkedStars = ui->checkBoxStarsEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setStarsEnabled(checkedStars);
    if (checkedStars)
    {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelTitleNumberOfStars->setEnabled(true);
        ui->sliderNumberOfStars->setEnabled(true);
        ui->labelNumberOfStars->setEnabled(true);
        ui->sliderStarSpeed->setEnabled(true);
        ui->labelTitleStarSpeed->setEnabled(true);
        ui->labelStarSpeed->setEnabled(true);
        ui->comboBoxStarsDirection->setEnabled(true);
        ui->labelTitleStarDirection->setEnabled(true);
    }
    else
    {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelTitleNumberOfStars->setEnabled(false);
        ui->sliderNumberOfStars->setEnabled(false);
        ui->labelNumberOfStars->setEnabled(false);
        ui->sliderStarSpeed->setEnabled(false);
        ui->labelTitleStarSpeed->setEnabled(false);
        ui->labelStarSpeed->setEnabled(false);
        ui->comboBoxStarsDirection->setEnabled(false);
        ui->labelTitleStarDirection->setEnabled(false);
    }
}


void settingsWindow::on_buttonColorReflection_clicked()
{
    QColor oldColor(mainWindow->getEffect()->getReflectionColor());
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->getEffect()->setScrollerReflectionColor(newColor.name());

        QString qss = QString("background-color: %1").arg(mainWindow->getEffect()->getReflectionColor());
        ui->buttonColorReflection->setStyleSheet(qss);
    }
}


void settingsWindow::on_buttonBrowseFont_clicked()
{
    if (ui->fontWidget->isHidden())
    {
        ui->fontWidget->setVisible(true);
    }
    else
    {
        ui->fontWidget->setVisible(false);
    }
}

void settingsWindow::loadBitmapFont(QString file)
{
    ui->fontWidget->setVisible(false);
    mainWindow->getEffect()->setScrollerFont(file);

    int extensionPos = file.lastIndexOf('.');
    QString thumb(file.left(extensionPos) + ".thumb.png");
    ui->buttonScrollerFontImage->setIcon(QIcon(thumb));
}

void settingsWindow::loadBitmapFontPrinter(QString file)
{
    ui->fontWidgetPrinter->setVisible(false);
    mainWindow->getEffect()->setPrinterFont(file);
    int extensionPos = file.lastIndexOf('.');
    QString thumb(file.left(extensionPos) + ".thumb.png");
    ui->buttonPrinterFontImage->setIcon(QIcon(thumb));
}

void settingsWindow::on_sliderReflectionOpacity_valueChanged(int value)
{
    mainWindow->getEffect()->setReflectionOpacity(value);
    ui->labelReflectionOpacity->setText(QString::number(value) + " %");
}


void settingsWindow::on_sliderVumeterOpacity_valueChanged(int value)
{
    mainWindow->getEffect()->setVumeterOpacity(value);
    ui->labelVumeterOpacity->setText(QString::number(value) + " %");
}


void settingsWindow::on_buttonBrowseFontPrinter_clicked()
{
    if (ui->fontWidgetPrinter->isHidden())
    {
        ui->fontWidgetPrinter->setVisible(true);
    }
    else
    {
        ui->fontWidgetPrinter->setVisible(false);
    }
}


void settingsWindow::on_sliderPrinterXScale_valueChanged(int value)
{
    mainWindow->getEffect()->setPrinterFontScaleX(value);
    ui->labelPrinterXScale->setText(QString::number(value));
}


void settingsWindow::on_sliderPrinterYScale_valueChanged(int value)
{
    mainWindow->getEffect()->setPrinterFontScaleY(value);
    ui->labelPrinterYScale->setText(QString::number(value));
}


void settingsWindow::on_checkBoxOnlyOneInstance_toggled(bool checked)
{
    mainWindow->setAllowOnlyOneInstanceEnabled(checked);
    ui->checkBoxEnqueueItems->setEnabled(checked);
    ui->labelEnqueueItems->setEnabled(checked);
    if (ui->checkBoxOnlyOneInstance->isChecked())
    {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-on"]);
        if (ui->checkBoxEnqueueItems->isChecked())
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-on"]);
        }
        else
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-off"]);
        }
    }
    else
    {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-off"]);
        if (ui->checkBoxEnqueueItems->isChecked())
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-on-disabled"]);
        }
        else
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-off-disabled"]);
        }
    }
}


void settingsWindow::on_sliderSilenceTimeOut_valueChanged(int value)
{
    QString sec = "seconds";
    if (value == 1)
    {
        sec = "second";
    }
    ui->labelSilenceTimeOut->setText(QString::number(value) + " " + sec);
}


void settingsWindow::on_checkBoxSilenceTimeout_toggled(bool checked)
{
    bool checkedSilenceTimeout = ui->checkBoxSilenceTimeout->checkState() == Qt::Checked ? true : false;
    if (checkedSilenceTimeout)
    {
        ui->checkBoxSilenceTimeout->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelSilenceTimeOut->setEnabled(true);
        ui->sliderSilenceTimeOut->setEnabled(true);
    }
    else
    {
        ui->checkBoxSilenceTimeout->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelSilenceTimeOut->setEnabled(false);
        ui->sliderSilenceTimeOut->setEnabled(false);
    }
}


void settingsWindow::on_buttonBrowseUADESonglengths_clicked()
{
    QString startFolder = ui->lineEditUADESonglength->text();
    if (startFolder.startsWith("/"))
    {
        startFolder = QApplication::applicationDirPath() + startFolder;
    }
    QString file = QFileDialog::getOpenFileName(this, "Choose your uade.md5", startFolder, "*.md5");
    if (!file.isEmpty())
    {
        ui->lineEditUADESonglength->setText(file);
    }
}


void settingsWindow::on_buttonSelectionColor_clicked()
{
    QColor oldColor(mainWindow->getColorSelection().left(7));
    QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
    if (newColor.isValid())
    {
        mainWindow->setColorSelection(newColor.name());

        QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonSelectionColor->setStyleSheet(qss);
    }
}


void settingsWindow::on_checkBoxMilliseconds_toggled(bool checked)
{
    mainWindow->setDisplayMilliseconds(checked);
    if (ui->checkBoxMilliseconds->isChecked())
    {
        ui->checkBoxMilliseconds->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxMilliseconds->setIcon(mainWindow->icons["checkbox-off"]);
    }
}


void settingsWindow::on_SliderStereoSeparationOpenMPT_valueChanged(int value)
{
    ui->labelStereoSeparationOpenMPT->setText(QString::number(value / 2) + " %");
}


void settingsWindow::on_checkBoxEnqueueItems_toggled(bool checked)
{
    mainWindow->setEnqueueItems(checked);
    if (ui->checkBoxEnqueueItems->isChecked())
    {
        ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-off"]);
    }
}


void settingsWindow::on_checkBoxVUMeterEnabled_toggled(bool checked)
{
    bool checkedVUMeters = ui->checkBoxVUMeterEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setVUMeterEnabled(checkedVUMeters);

    if (checkedVUMeters)
    {
        ui->checkBoxVUMeterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelVUMeterTitleBottomColor->setEnabled(true);
        ui->labelVUMeterTitleTopColor->setEnabled(true);
        ui->labelVUMeterTitleMiddleColor->setEnabled(true);
        ui->buttonColorVUMeterTop->setEnabled(true);
        ui->buttonColorVUMeterMiddle->setEnabled(true);
        ui->buttonColorVUMeterBottom->setEnabled(true);
        ui->labelVUMeterTitleWidth->setEnabled(true);
        ui->labelVumeterTitleOpacity->setEnabled(true);
        ui->sliderVUMeterWidth->setEnabled(true);
        ui->sliderVumeterOpacity->setEnabled(true);
        ui->labelVUMeterWidth->setEnabled(true);
        ui->labelVumeterOpacity->setEnabled(true);
        ui->checkBoxPeaks->setEnabled(true);
        ui->labelVUMeterTitleFallingPeaks->setEnabled(true);

        bool checkedPeaks = ui->checkBoxPeaks->checkState() == Qt::Checked ? true : false;
        if (checkedPeaks)
        {
            ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-on"]);
            ui->labelVUMeterPeakColor->setEnabled(true);
            ui->labelTitleVUMeterPeak->setEnabled(true);
            ui->buttonColorVUMeterPeak->setEnabled(true);
            ui->sliderPeakHeight->setEnabled(true);
            ui->labelVUMeterPeakHeight->setEnabled(true);
        }
        else
        {
            ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-off"]);
            ui->labelVUMeterPeakColor->setEnabled(false);
            ui->labelTitleVUMeterPeak->setEnabled(false);
            ui->buttonColorVUMeterPeak->setEnabled(false);
            ui->sliderPeakHeight->setEnabled(false);
            ui->labelVUMeterPeakHeight->setEnabled(false);
        }
    }
    else
    {
        ui->checkBoxVUMeterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelVUMeterTitleBottomColor->setEnabled(false);
        ui->labelVUMeterTitleTopColor->setEnabled(false);
        ui->labelVUMeterTitleMiddleColor->setEnabled(false);
        ui->buttonColorVUMeterTop->setEnabled(false);
        ui->buttonColorVUMeterMiddle->setEnabled(false);
        ui->buttonColorVUMeterBottom->setEnabled(false);
        ui->sliderPeakHeight->setEnabled(false);
        ui->labelVUMeterPeakHeight->setEnabled(false);
        ui->labelVUMeterTitleWidth->setEnabled(false);
        ui->labelVumeterTitleOpacity->setEnabled(false);
        ui->sliderVUMeterWidth->setEnabled(false);
        ui->sliderVumeterOpacity->setEnabled(false);
        ui->labelVUMeterWidth->setEnabled(false);
        ui->labelVumeterOpacity->setEnabled(false);

        ui->checkBoxPeaks->setEnabled(false);
        ui->labelVUMeterTitleFallingPeaks->setEnabled(false);
        ui->labelVUMeterPeakColor->setEnabled(false);
        ui->labelTitleVUMeterPeak->setEnabled(false);
        ui->checkBoxPeaks->setEnabled(false);
        ui->buttonColorVUMeterPeak->setEnabled(false);
        ui->sliderPeakHeight->setEnabled(false);
        ui->labelVUMeterPeakHeight->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxScrollerEnabled_toggled(bool checked)
{
    bool checkedScroller = ui->checkBoxScrollerEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setScrollerEnabled(checkedScroller);
    if (checkedScroller)
    {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelTitleAmplitude->setEnabled(true);
        ui->sliderAmplitude->setEnabled(true);
        ui->labelAmplitude->setEnabled(true);
        ui->labelTitleFrequency->setEnabled(true);
        ui->sliderFrequency->setEnabled(true);
        ui->labelFrequency->setEnabled(true);
        ui->labelTitleSinusSpeed->setEnabled(true);
        ui->sliderSinusSpeed->setEnabled(true);
        ui->labelSinusSpeed->setEnabled(true);
        ui->labelTitleScrollSpeed->setEnabled(true);
        ui->sliderScrollSpeed->setEnabled(true);
        ui->labelScrollSpeed->setEnabled(true);
        ui->labelTitleVerticalScrollPosition->setEnabled(true);
        ui->sliderVerticalScrollPosition->setEnabled(true);
        ui->labelVerticalScrollPosition->setEnabled(true);
        ui->labelTitleScrollerXScale->setEnabled(true);
        ui->sliderScrollerXScale->setEnabled(true);
        ui->labelScrollerXScale->setEnabled(true);
        ui->labelTitleScrollerYScale->setEnabled(true);
        ui->sliderScrollerYScale->setEnabled(true);
        ui->labelScrollerYScale->setEnabled(true);
        ui->labelTitleFont->setEnabled(true);
        ui->labelTitleSinusFontScaling->setEnabled(true);
        ui->checkBoxSinusFontScaling->setEnabled(true);
    }
    else
    {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelTitleAmplitude->setEnabled(false);
        ui->sliderAmplitude->setEnabled(false);
        ui->labelAmplitude->setEnabled(false);
        ui->labelTitleFrequency->setEnabled(false);
        ui->sliderFrequency->setEnabled(false);
        ui->labelFrequency->setEnabled(false);
        ui->labelTitleSinusSpeed->setEnabled(false);
        ui->sliderSinusSpeed->setEnabled(false);
        ui->labelSinusSpeed->setEnabled(false);
        ui->labelTitleScrollSpeed->setEnabled(false);
        ui->sliderScrollSpeed->setEnabled(false);
        ui->labelScrollSpeed->setEnabled(false);
        ui->labelTitleVerticalScrollPosition->setEnabled(false);
        ui->sliderVerticalScrollPosition->setEnabled(false);
        ui->labelVerticalScrollPosition->setEnabled(false);
        ui->labelTitleScrollerXScale->setEnabled(false);
        ui->sliderScrollerXScale->setEnabled(false);
        ui->labelScrollerXScale->setEnabled(false);
        ui->labelTitleScrollerYScale->setEnabled(false);
        ui->sliderScrollerYScale->setEnabled(false);
        ui->labelScrollerYScale->setEnabled(false);
        ui->labelTitleFont->setEnabled(false);
        ui->labelTitleSinusFontScaling->setEnabled(false);
        ui->checkBoxSinusFontScaling->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxPrinterEnabled_toggled(bool checked)
{
    bool checkedPrinter = ui->checkBoxPrinterEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setPrinterEnabled(checkedPrinter);
    if (checkedPrinter)
    {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->sliderPrinterXScale->setEnabled(true);
        ui->sliderPrinterYScale->setEnabled(true);
        ui->labelTitlePrinterXScale->setEnabled(true);
        ui->labelTitlePrinterYScale->setEnabled(true);
        ui->labelPrinterXScale->setEnabled(true);
        ui->labelPrinterYScale->setEnabled(true);
        ui->labelTitlePrinterFont->setEnabled(true);
    }
    else
    {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->sliderPrinterXScale->setEnabled(false);
        ui->sliderPrinterYScale->setEnabled(false);
        ui->labelTitlePrinterXScale->setEnabled(false);
        ui->labelTitlePrinterYScale->setEnabled(false);
        ui->labelPrinterXScale->setEnabled(false);
        ui->labelPrinterYScale->setEnabled(false);
        ui->labelTitlePrinterFont->setEnabled(false);
    }
}


void settingsWindow::on_sliderNumberOfStars_valueChanged(int value)
{
    mainWindow->getEffect()->setNumberOfStars(value);
    ui->labelNumberOfStars->setText(QString::number(value));
}


void settingsWindow::on_sliderStarSpeed_valueChanged(int value)
{
    mainWindow->getEffect()->setStarSpeed(value);
    ui->labelStarSpeed->setText(QString::number(value));
}


void settingsWindow::on_comboBoxStarsDirection_textActivated(const QString& arg1)
{
    QString selected = ui->comboBoxStarsDirection->itemData(ui->comboBoxStarsDirection->currentIndex()).toString();
    mainWindow->getEffect()->setStarsDirection(selected);
}


void settingsWindow::on_checkBoxSinusFontScaling_toggled(bool checked)
{
    if (ui->checkBoxSinusFontScaling->isChecked())
    {
        ui->checkBoxSinusFontScaling->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSinusFontScaling->setIcon(mainWindow->icons["checkbox-off"]);
    }
    mainWindow->getEffect()->setSinusFontScalingEnabled(checked);
}


void settingsWindow::on_buttonScrollerFontImage_clicked()
{
    if (ui->fontWidget->isHidden())
    {
        ui->fontWidget->setVisible(true);
    }
    else
    {
        ui->fontWidget->setVisible(false);
    }
}


void settingsWindow::on_buttonPrinterFontImage_clicked()
{
    if (ui->fontWidgetPrinter->isHidden())
    {
        ui->fontWidgetPrinter->setVisible(true);
    }
    else
    {
        ui->fontWidgetPrinter->setVisible(false);
    }
}


void settingsWindow::on_sliderResolutionWidth_valueChanged(int value)
{
    mainWindow->getEffect()->setResolutionWidth(value);
    ui->labelResolutionWidth->setText(QString::number(value));
}


void settingsWindow::on_sliderResolutionHeight_valueChanged(int value)
{
    mainWindow->getEffect()->setResolutionHeight(value);
    ui->labelResolutionHeight->setText(QString::number(value));
}


void settingsWindow::on_checkBoxAspectRatio_toggled(bool checked)
{
    bool checkedAspectRatio = ui->checkBoxAspectRatio->checkState() == Qt::Checked ? true : false;
    if (checkedAspectRatio)
    {
        ui->checkBoxAspectRatio->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxAspectRatio->setIcon(mainWindow->icons["checkbox-off"]);
    }
    mainWindow->getEffect()->setKeepAspectRatio(checkedAspectRatio);
}


void settingsWindow::on_comboBoxSidSongFileUpdateFrequency_textActivated(const QString& arg1)
{
    QString selected = ui->comboBoxSidSongFileUpdateFrequency->itemData(
        ui->comboBoxSidSongFileUpdateFrequency->currentIndex()).toString();
    mainWindow->setSIDSongLengthFrequency(selected);
}


void settingsWindow::on_buttonDownloadSIDLength_clicked()
{
    QUrl imageUrl("https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/Songlengths.md5");
    mainWindow->filedownloader = new FileDownloader(imageUrl, this);
    ui->buttonDownloadSIDLength->setEnabled(true);
    ui->buttonDownloadSIDLength->setText("Downloading...");

    connect(mainWindow->filedownloader, SIGNAL(downloaded()), this, SLOT(downloadComplete()));
}

void settingsWindow::downloadComplete()
{
    if (mainWindow->filedownloader->downloadedData().size() > 0)
    {
        QFile file(QApplication::applicationDirPath() + QDir::separator() + "user/plugin/sid/Songlengths.md5");
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << mainWindow->filedownloader->downloadedData();
            file.close();
            QDateTime::currentDateTime().toSecsSinceEpoch();
            QSettings settings(QApplication::applicationDirPath() + QDir::separator() + "user/settings.ini",
                               QSettings::IniFormat);
            qint64 seconds = QDateTime::currentDateTime().toSecsSinceEpoch();
            settings.setValue("libsidplayfp/timesidsonglengthdownloaded", seconds);
            settings.setValue("libsidplayfp/sidsongpath",
                              QApplication::applicationDirPath() + QDir::separator() +
                              "user/plugin/sid/Songlengths.md5");
            mainWindow->SIDSongLengthDownloadedEpoch = seconds;
            mainWindow->addDebugText(
                "Downloaded " + mainWindow->filedownloader->getUrl().toString() + " to " + file.fileName());
            mainWindow->setSIDSongPathDownloaded(
                QApplication::applicationDirPath() + QDir::separator() + "user/plugin/sid/Songlengths.md5");
            QDateTime qdt = QDateTime::fromSecsSinceEpoch(mainWindow->getSIDSongLengthDownloaded());
            ui->labelSIDSongLengthDownloaded->setText(
                "Downloaded official Songlengths.md5 to " + mainWindow->getSIDSongPathDownloaded() + " at " + qdt.
                toString("yyyy-MM-dd hh:mm:ss"));
            ui->buttonDownloadSIDLength->setEnabled(true);
            ui->buttonDownloadSIDLength->setText("Download now");
        }
        else
        {
            mainWindow->addDebugText("Couldn't write to file " + file.fileName());
        }
    }
    else
    {
        mainWindow->addDebugText("Failed to download " + mainWindow->filedownloader->getUrl().toString());
    }
}

void settingsWindow::on_sliderNumberOfRasterBars_valueChanged(int value)
{
    mainWindow->getEffect()->setNumberOfRasterBars(value);
    ui->labelNumberOfRasterBars->setText(QString::number(value));
}


void settingsWindow::on_sliderRasterBarsVerticalSpacing_valueChanged(int value)
{
    mainWindow->getEffect()->setRasterBarsVerticalSpacing(value);
    ui->labelRasterBarsVerticalSpacing->setText(QString::number(value));
}


void settingsWindow::on_sliderRasterBarsBarHeight_valueChanged(int value)
{
    mainWindow->getEffect()->setRasterBarsBarHeight(value);
    ui->labelRasterBarsBarHeight->setText(QString::number(value));
}


void settingsWindow::on_sliderRasterBarsSpeed_valueChanged(int value)
{
    mainWindow->getEffect()->setRasterBarsSpeed(value);
    ui->labelRasterBarsSpeed->setText(QString::number(value));
}

void settingsWindow::on_sliderRasterbarsOpacity_valueChanged(int value)
{
    mainWindow->getEffect()->setRasterBarsOpacity(value);
    ui->labelRasterbarsOpacity->setText(QString::number(value));
}


void settingsWindow::on_checkBoxRasterbarsEnabled_toggled(bool checked)
{
    bool checkedRasterbars = ui->checkBoxRasterbarsEnabled->checkState() == Qt::Checked ? true : false;
    mainWindow->getEffect()->setRasterBarsEnabled(checkedRasterbars);
    if (checkedRasterbars)
    {
        ui->checkBoxRasterbarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelTitleNumberOfRasterBars->setEnabled(true);
        ui->sliderNumberOfRasterBars->setEnabled(true);
        ui->labelNumberOfRasterBars->setEnabled(true);
        ui->labelTitleRasterBarsBarHeight->setEnabled(true);
        ui->sliderRasterBarsBarHeight->setEnabled(true);
        ui->labelRasterBarsBarHeight->setEnabled(true);
        ui->labelTitleRasterBarsVerticalSpacing->setEnabled(true);
        ui->sliderRasterBarsVerticalSpacing->setEnabled(true);
        ui->labelRasterBarsVerticalSpacing->setEnabled(true);
        ui->labelTitleRasterBarsSpeed->setEnabled(true);
        ui->sliderRasterBarsSpeed->setEnabled(true);
        ui->labelRasterBarsSpeed->setEnabled(true);
        ui->labelTitleRasterbarsOpacity->setEnabled(true);
        ui->sliderRasterbarsOpacity->setEnabled(true);
        ui->labelRasterbarsOpacity->setEnabled(true);
    }
    else
    {
        ui->checkBoxRasterbarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelTitleNumberOfRasterBars->setEnabled(false);
        ui->sliderNumberOfRasterBars->setEnabled(false);
        ui->labelNumberOfRasterBars->setEnabled(false);
        ui->labelTitleRasterBarsBarHeight->setEnabled(false);
        ui->sliderRasterBarsBarHeight->setEnabled(false);
        ui->labelRasterBarsBarHeight->setEnabled(false);
        ui->labelTitleRasterBarsVerticalSpacing->setEnabled(false);
        ui->sliderRasterBarsVerticalSpacing->setEnabled(false);
        ui->labelRasterBarsVerticalSpacing->setEnabled(false);
        ui->labelTitleRasterBarsSpeed->setEnabled(false);
        ui->sliderRasterBarsSpeed->setEnabled(false);
        ui->labelRasterBarsSpeed->setEnabled(false);
        ui->labelTitleRasterbarsOpacity->setEnabled(false);
        ui->sliderRasterbarsOpacity->setEnabled(false);
        ui->labelRasterbarsOpacity->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxCustomScrolltextEnabled_toggled(bool checked)
{
    bool checkedCustomScrolltextEnabled = ui->checkBoxCustomScrolltextEnabled->checkState() == Qt::Checked
                                              ? true
                                              : false;
    if (checkedCustomScrolltextEnabled)
    {
        ui->checkBoxCustomScrolltextEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxCustomScrolltextEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    mainWindow->getEffect()->setCustomScrolltextEnabled(checkedCustomScrolltextEnabled);
    ui->textEditCustomScrolltext->setEnabled(checkedCustomScrolltextEnabled);
}

void settingsWindow::updateCheckBoxes()
{
    if (ui->checkBoxCustomScrolltextEnabled->isChecked())
    {
        ui->checkBoxCustomScrolltextEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxCustomScrolltextEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxSinusFontScaling->isChecked())
    {
        ui->checkBoxSinusFontScaling->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSinusFontScaling->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxPeaks->isChecked())
    {
        ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxPeaks->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxReflectionEnabled->isChecked())
    {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxScrollerEnabled->isChecked())
    {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxRasterbarsEnabled->isChecked())
    {
        ui->checkBoxRasterbarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxRasterbarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxPrinterEnabled->isChecked())
    {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxStarsEnabled->isChecked())
    {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxAspectRatio->isChecked())
    {
        ui->checkBoxAspectRatio->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxAspectRatio->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxVUMeterEnabled->isChecked())
    {
        ui->checkBoxVUMeterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxVUMeterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxFilterOpenMPT->isChecked())
    {
        ui->checkBoxFilterOpenMPT->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxFilterOpenMPT->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxFilterEnabled->isChecked())
    {
        ui->checkBoxFilterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxFilterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxSilenceTimeout->isChecked())
    {
        ui->checkBoxSilenceTimeout->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSilenceTimeout->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxSongLengthUADE->isChecked())
    {
        ui->checkBoxSongLengthUADE->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSongLengthUADE->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxSIDSonglengthsEnabled->isChecked())
    {
        ui->checkBoxSIDSonglengthsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSIDSonglengthsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxOnlyOneInstance->isChecked())
    {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxSystrayOnQuit->isChecked())
    {
        ui->checkBoxSystrayOnQuit->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSystrayOnQuit->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxNormalizer->isChecked())
    {
        ui->checkBoxNormalizer->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxNormalizer->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxReverb->isChecked())
    {
        ui->checkBoxReverb->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxReverb->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxEnqueueItems->isChecked())
    {
        ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxResetVolume->isChecked())
    {
        ui->checkBoxResetVolume->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxResetVolume->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (ui->checkBoxMilliseconds->isChecked())
    {
        ui->checkBoxMilliseconds->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxMilliseconds->setIcon(mainWindow->icons["checkbox-off"]);
    }
    if (!ui->checkBoxOnlyOneInstance->isChecked())
    {
        if (ui->checkBoxEnqueueItems->isChecked())
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-on-disabled"]);
        }
        else
        {
            ui->checkBoxEnqueueItems->setIcon(mainWindow->icons["checkbox-off-disabled"]);
        }
    }
}

void settingsWindow::forceUpdateToSliders()
{
    on_sliderVumeterOpacity_valueChanged(ui->sliderVumeterOpacity->value());
    on_sliderAmplitude_valueChanged(ui->sliderAmplitude->value());
    on_sliderFrequency_valueChanged(ui->sliderFrequency->value());
    on_sliderSinusSpeed_valueChanged(ui->sliderSinusSpeed->value());
    on_sliderScrollSpeed_valueChanged(ui->sliderScrollSpeed->value());
    on_sliderVerticalScrollPosition_valueChanged(ui->sliderVerticalScrollPosition->value());
    on_sliderScrollerXScale_valueChanged(ui->sliderScrollerXScale->value());
    on_sliderScrollerYScale_valueChanged(ui->sliderScrollerYScale->value());
    on_sliderPrinterXScale_valueChanged(ui->sliderPrinterXScale->value());
    on_sliderPrinterYScale_valueChanged(ui->sliderPrinterYScale->value());
    on_sliderPeakHeight_valueChanged(ui->sliderPeakHeight->value());
    on_sliderReflectionOpacity_valueChanged(ui->sliderReflectionOpacity->value());
    on_sliderPlaylistsRowHeight_valueChanged(ui->sliderPlaylistsRowHeight->value());
    on_sliderRowHeight_valueChanged(ui->sliderRowHeight->value());
    on_sliderNowPlayingFontSize_valueChanged(ui->sliderNowPlayingFontSize->value());
    on_sliderResetVolumeToValue_valueChanged(ui->sliderResetVolumeToValue->value());
    on_SliderNormalizerFadeTim_valueChanged(ui->SliderNormalizerFadeTim->value());
    on_SliderNormalizerMaxAmp_valueChanged(ui->SliderNormalizerMaxAmp->value());
    on_SliderNormalizerThreshold_valueChanged(ui->SliderNormalizerThreshold->value());
    on_sliderNumberOfStars_valueChanged(ui->sliderNumberOfStars->value());
    on_sliderStarSpeed_valueChanged(ui->sliderStarSpeed->value());
    on_sliderResolutionWidth_valueChanged(ui->sliderResolutionWidth->value());
    on_sliderResolutionHeight_valueChanged(ui->sliderResolutionHeight->value());
    on_sliderRasterBarsBarHeight_valueChanged(ui->sliderRasterBarsBarHeight->value());
    on_sliderRasterBarsSpeed_valueChanged(ui->sliderRasterBarsSpeed->value());
    on_sliderRasterbarsOpacity_valueChanged(ui->sliderRasterbarsOpacity->value());
    on_sliderRasterBarsVerticalSpacing_valueChanged(ui->sliderRasterBarsVerticalSpacing->value());
    on_sliderNumberOfRasterBars_valueChanged(ui->sliderNumberOfRasterBars->value());
}


void settingsWindow::on_checkBoxSongLengthUADE_toggled(bool checked)
{
    bool checkedSongLengthUADE = ui->checkBoxSongLengthUADE->checkState() == Qt::Checked ? true : false;
    if (checkedSongLengthUADE)
    {
        ui->checkBoxSongLengthUADE->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelUADESongFilePath->setEnabled(true);
        ui->lineEditUADESonglength->setEnabled(true);
        ui->buttonBrowseUADESonglengths->setEnabled(true);
    }
    else
    {
        ui->checkBoxSongLengthUADE->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelUADESongFilePath->setEnabled(false);
        ui->lineEditUADESonglength->setEnabled(false);
        ui->buttonBrowseUADESonglengths->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxSIDSonglengthsEnabled_toggled(bool checked)
{
    ui->labelSidSongFilePath->setEnabled(checked);
    ui->lineEditSonglength->setEnabled(checked);
    ui->labelSidSongFilePathOld->setEnabled(checked);
    ui->lineEditSonglengthOld->setEnabled(checked);
    ui->buttonBrowseSonglengths->setEnabled(checked);
    ui->buttonBrowseSonglengthsOld->setEnabled(checked);
    if (ui->checkBoxSIDSonglengthsEnabled->isChecked())
    {
        ui->checkBoxSIDSonglengthsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSIDSonglengthsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
}


void settingsWindow::on_checkBoxFilterOpenMPT_toggled(bool checked)
{
    bool checkedFilterOpenMPT = ui->checkBoxFilterOpenMPT->checkState() == Qt::Checked ? true : false;
    if (checkedFilterOpenMPT)
    {
        ui->checkBoxFilterOpenMPT->setIcon(mainWindow->icons["checkbox-on"]);
        ui->label_50->setEnabled(true);
        ui->comboBoxFilterOpenMPT->setEnabled(true);
    }
    else
    {
        ui->checkBoxFilterOpenMPT->setIcon(mainWindow->icons["checkbox-off"]);
        ui->label_50->setEnabled(false);
        ui->comboBoxFilterOpenMPT->setEnabled(false);
    }
}


void settingsWindow::on_checkBoxOnlyOneInstance_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("You need to close all instances of BZR Player for this setting to have effect.");
    msgBox.exec();
}


void settingsWindow::on_checkBoxFilterEnabled_toggled(bool checked)
{
    if (ui->checkBoxFilterEnabled->isChecked())
    {
        ui->checkBoxFilterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxFilterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
    }
}


void settingsWindow::on_checkBoxSystrayOnQuit_toggled(bool checked)
{
    if (ui->checkBoxSystrayOnQuit->isChecked())
    {
        ui->checkBoxSystrayOnQuit->setIcon(mainWindow->icons["checkbox-on"]);
    }
    else
    {
        ui->checkBoxSystrayOnQuit->setIcon(mainWindow->icons["checkbox-off"]);
    }
}


void settingsWindow::on_sliderNowPlayingFontSize_valueChanged(int value)
{
    ui->labelNowPlayingFontSize->setText(QString::number(value) + " pixels");
    mainWindow->setNowPlayingFontSize(value);
}
