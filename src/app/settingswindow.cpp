#include <fstream>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include "album.h"
#include "fmod_common.h"
#include "plugins.h"
#include "settingswindow.h"
#include "ui_settingswindow.h"

settingsWindow::settingsWindow(QWidget *parent) : QDialog(parent),
                                                  ui(new Ui::settingsWindow) {
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    ui->setupUi(this);
    mainWindow = static_cast<MainWindow *>(this->parent());
    connect(ui->textEditScrollerCustomText, SIGNAL(textChanged()), this, SLOT(updateScrollText()));

    ui->sliderVuMeterWidth->installEventFilter(this);
    ui->sliderVuMeterOpacity->installEventFilter(this);
    ui->sliderScrollerAmplitude->installEventFilter(this);
    ui->sliderScrollerFrequency->installEventFilter(this);
    ui->sliderScrollerSinusSpeed->installEventFilter(this);
    ui->sliderScrollerScrollSpeed->installEventFilter(this);
    ui->sliderScrollerVerticalPosition->installEventFilter(this);
    ui->sliderScrollerFontXScale->installEventFilter(this);
    ui->sliderScrollerFontYScale->installEventFilter(this);
    ui->sliderPrinterFontXScale->installEventFilter(this);
    ui->sliderPrinterFontYScale->installEventFilter(this);
    ui->sliderVuMeterPeakHeight->installEventFilter(this);
    ui->sliderReflectionOpacity->installEventFilter(this);
    ui->sliderAppearancePlaylistRowHeight->installEventFilter(this);
    ui->sliderAppearancePlaylistItemRowHeight->installEventFilter(this);
    ui->sliderAppearanceNowPlayingFontSize->installEventFilter(this);
    ui->sliderDefaultAudioLevel->installEventFilter(this);
    ui->sliderNormalizerFadeTime->installEventFilter(this);
    ui->sliderNormalizerMaxAmp->installEventFilter(this);
    ui->sliderNormalizerThreshold->installEventFilter(this);
    ui->sliderStarfieldAmount->installEventFilter(this);
    ui->sliderStarfieldSpeed->installEventFilter(this);
    ui->sliderVisualizerResolutionWidth->installEventFilter(this);
    ui->sliderVisualizerResolutionHeight->installEventFilter(this);
    ui->sliderRasterBarsHeight->installEventFilter(this);
    ui->sliderRasterBarsSpeed->installEventFilter(this);
    ui->sliderRasterBarsOpacity->installEventFilter(this);
    ui->sliderRasterBarsVerticalSpacing->installEventFilter(this);
    ui->sliderRasterBarsAmount->installEventFilter(this);
    ui->sliderRotatingObjectFocalLength->installEventFilter(this);
    ui->sliderRotatingObjectModelSize->installEventFilter(this);
    ui->sliderRotatingObjectOrbitSize->installEventFilter(this);
    ui->sliderRotatingObjectOrbitSpeed->installEventFilter(this);

    ui->comboBoxAdPlugEmulator->installEventFilter(this);
    ui->comboBoxAdPlugEmulator->addItem("Tatsuyuki Satoh", "0");
    ui->comboBoxAdPlugEmulator->addItem("Ken Silverman", "1");
    ui->comboBoxAdPlugEmulator->addItem("Woody (DOSBox)", "2");
    ui->comboBoxAdPlugEmulator->addItem("Nuked OPL3", "3");

    ui->comboBoxAdPlugFreq->installEventFilter(this);
    ui->comboBoxAdPlugFreq->addItem("11025", "11025");
    ui->comboBoxAdPlugFreq->addItem("22050", "22050");
    ui->comboBoxAdPlugFreq->addItem("44100", "44100");

    ui->comboBoxAdPlugPlayback->installEventFilter(this);
    ui->comboBoxAdPlugPlayback->addItem("Mono", "0");
    ui->comboBoxAdPlugPlayback->addItem("Stereo", "1");
    ui->comboBoxAdPlugPlayback->addItem("Surround", "2");

    ui->comboBoxHivelyTrackerStereoSeparation->installEventFilter(this);
    ui->comboBoxHivelyTrackerStereoSeparation->addItem("0% (Mono)", "0");
    ui->comboBoxHivelyTrackerStereoSeparation->addItem("25%", "1");
    ui->comboBoxHivelyTrackerStereoSeparation->addItem("50%", "2");
    ui->comboBoxHivelyTrackerStereoSeparation->addItem("75%", "3");
    ui->comboBoxHivelyTrackerStereoSeparation->addItem("100% (Paula)", "4");

    ui->comboBoxLibopenmptFilter->installEventFilter(this);
    ui->comboBoxLibopenmptFilter->addItem("Auto", "auto");
    ui->comboBoxLibopenmptFilter->addItem("A500 Filter", "a500");
    ui->comboBoxLibopenmptFilter->addItem("A1200 Filter", "a1200");
    ui->comboBoxLibopenmptFilter->addItem("Unfiltered", "unfiltered");

    ui->comboBoxLibopenmptResampling->installEventFilter(this);
    ui->comboBoxLibopenmptResampling->addItem("Internal Default", "0");
    ui->comboBoxLibopenmptResampling->addItem("No Interpolation (1 tap)", "1");
    ui->comboBoxLibopenmptResampling->addItem("Linear (2 taps)", "2");
    ui->comboBoxLibopenmptResampling->addItem("Cubic (4 taps)", "4");
    ui->comboBoxLibopenmptResampling->addItem("Sinc (8 taps)", "8");

    ui->comboBoxLibopenmptDither->installEventFilter(this);
    ui->comboBoxLibopenmptDither->addItem("No dithering", "0");
    ui->comboBoxLibopenmptDither->addItem("Default", "1");
    ui->comboBoxLibopenmptDither->addItem("Rectangular, 0.5 bit depth", "2");
    ui->comboBoxLibopenmptDither->addItem("Rectangular, 1 bit depth", "3");

    ui->sliderLibopenmptStereoSeparation->installEventFilter(this);

    ui->comboBoxUadeFreq->installEventFilter(this);
    ui->comboBoxUadeFreq->addItem("11025", "11025");
    ui->comboBoxUadeFreq->addItem("22050", "22050");
    ui->comboBoxUadeFreq->addItem("32000", "32000");
    ui->comboBoxUadeFreq->addItem("44100", "44100");
    ui->comboBoxUadeFreq->addItem("48000", "48000");

    ui->comboBoxUadeResampler->installEventFilter(this);
    ui->comboBoxUadeResampler->addItem("None", "none");
    ui->comboBoxUadeResampler->addItem("Default", "default");
    ui->comboBoxUadeResampler->addItem("Sinc (best for freqs > 44100)", "sinc");

    ui->sliderUadePanning->installEventFilter(this);

    ui->sliderUadeSilenceTimeOut->installEventFilter(this);

    ui->comboBoxUadeFilterEmuMode->installEventFilter(this);
    ui->comboBoxUadeFilterEmuMode->addItem("A500", "a500");
    ui->comboBoxUadeFilterEmuMode->addItem("A1200", "a1200");

    ui->comboBoxUadeLedFilter->installEventFilter(this);
    ui->comboBoxUadeLedFilter->addItem("Auto", "Auto");
    ui->comboBoxUadeLedFilter->addItem("Always On", "Always On");
    ui->comboBoxUadeLedFilter->addItem("Always Off", "Always Off");

    ui->comboBoxOutputDevice->installEventFilter(this);
    ui->comboBoxReverb->installEventFilter(this);

    ui->comboBoxOutputDevice->addItem("Default", FMOD_OUTPUTTYPE_AUTODETECT);
    ui->comboBoxOutputDevice->addItem("No Sound", FMOD_OUTPUTTYPE_NOSOUND);
    ui->comboBoxOutputDevice->addItem("WAV Writer", FMOD_OUTPUTTYPE_WAVWRITER);
#ifdef WIN32
    ui->comboBoxOutputDevice->addItem("WASAPI", FMOD_OUTPUTTYPE_WASAPI);
    ui->comboBoxOutputDevice->addItem("ASIO 2.0", FMOD_OUTPUTTYPE_ASIO);
    ui->comboBoxOutputDevice->addItem("Windows Sonic", FMOD_OUTPUTTYPE_WINSONIC);
#else
    ui->comboBoxOutputDevice->addItem("ALSA", FMOD_OUTPUTTYPE_PULSEAUDIO);
    ui->comboBoxOutputDevice->addItem("PulseAudio", FMOD_OUTPUTTYPE_ALSA);
#endif

    ui->comboBoxDefaultPlayMode->installEventFilter(this);
    ui->comboBoxDefaultPlayMode->addItem("Last used", -1);
    ui->comboBoxDefaultPlayMode->addItem("Repeat disabled", mainWindow->normal);
    ui->comboBoxDefaultPlayMode->addItem("Repeat all", mainWindow->repeatPlaylist);
    ui->comboBoxDefaultPlayMode->addItem("Repeat one", mainWindow->repeatSong);

    int index = ui->comboBoxOutputDevice->findData(mainWindow->getOutputDevice());
    ui->comboBoxOutputDevice->setCurrentIndex(index);

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

    ui->checkBoxDefaultTrackLength->setChecked(mainWindow->getDefaultTrackLengthEnabled());

    ui->sliderDefaultTrackLength->installEventFilter(this);
    ui->sliderDefaultTrackLength->setValue(mainWindow->getDefaultTrackLengthValue());
    on_sliderDefaultTrackLength_valueChanged(mainWindow->getDefaultTrackLengthValue());

    ui->checkBoxShowLoopPoints->setChecked(mainWindow->getShowCheckBoxLoopPoints());
    mainWindow->showCheckBoxLoopPoints(mainWindow->getShowCheckBoxLoopPoints());

    ui->checkBoxDefaultAudioLevel->setChecked(mainWindow->getResetVolume());
    ui->sliderDefaultAudioLevel->setValue(mainWindow->getResetVolumeValue());

    const int comboBoxDefaultPlayModeIndex = ui->comboBoxDefaultPlayMode->findData(mainWindow->getDefaultPlayMode());
    ui->comboBoxDefaultPlayMode->setCurrentIndex(comboBoxDefaultPlayModeIndex);

    ui->checkBoxNormalizer->setChecked(mainWindow->isNormalizeEnabled());
    mainWindow->
            addDebugText("mainWindow->getNormalizeEnabled(): " + QString::number(mainWindow->isNormalizeEnabled()));

    ui->checkBoxReverb->setChecked(mainWindow->isReverbEnabled());
    mainWindow->addDebugText("mainWindow->getReverbEnabled(): " + QString::number(mainWindow->isReverbEnabled()));

    ui->checkBoxMilliseconds->setChecked(mainWindow->getDisplayMilliseconds());

    ui->checkBoxSystray->setChecked(mainWindow->getSystrayChecked());
    ui->checkBoxMinimizeToSystray->setChecked(mainWindow->getSystrayOnMinimizeChecked());
    ui->checkBoxMinimizeToSystray->setEnabled(mainWindow->getSystrayChecked());

    ui->checkBoxMenuBarHidden->setChecked(mainWindow->getMenuBarHiddenChecked());

    ui->sliderNormalizerFadeTime->setValue(mainWindow->getNormalizeFadeTime());
    ui->sliderNormalizerThreshold->setValue(mainWindow->getNormalizeThreshold());
    ui->sliderNormalizerMaxAmp->setValue(mainWindow->getNormalizeMaxAmp());

    ui->lineEditIgnoreSuffixes->setText(mainWindow->getIgnoreSuffix());
    ui->lineEditIgnorePrefixes->setText(mainWindow->getIgnorePrefix());

    ui->sliderAppearancePlaylistItemRowHeight->setValue(mainWindow->getPlaylistRowHeight());
    ui->sliderAppearancePlaylistRowHeight->setValue(mainWindow->getPlaylistsRowHeight());
    ui->sliderAppearanceNowPlayingFontSize->setValue(mainWindow->getNowPlayingFontSize());

    ui->sliderVuMeterWidth->setValue(mainWindow->getEffect()->getVuMeterWidth());
    ui->sliderVuMeterOpacity->setValue(mainWindow->getEffect()->getVuMeterOpacity());
    ui->sliderVuMeterPeakHeight->setValue(mainWindow->isVuMeterPeaksHeight());
    ui->checkBoxVuMeterPeaks->setChecked(mainWindow->isVuMeterPeaksEnabled());
    ui->sliderScrollerAmplitude->setValue(mainWindow->getEffect()->getAmplitude());
    ui->sliderScrollerFrequency->setValue(mainWindow->getEffect()->getFrequency() * 10000);
    ui->sliderScrollerSinusSpeed->setValue(mainWindow->getEffect()->getSinusSpeed() * 100);
    ui->sliderScrollerScrollSpeed->setValue(mainWindow->getEffect()->getScrollSpeed());
    ui->sliderScrollerVerticalPosition->setValue(mainWindow->getEffect()->getVerticalScrollPosition());
    ui->sliderScrollerFontXScale->setValue(mainWindow->getEffect()->getFontScaleX());
    ui->sliderScrollerFontYScale->setValue(mainWindow->getEffect()->getFontScaleY());
    ui->sliderPrinterFontXScale->setValue(mainWindow->getEffect()->getPrinterFontScaleX());
    ui->sliderPrinterFontYScale->setValue(mainWindow->getEffect()->getPrinterFontScaleY());
    ui->checkBoxReflectionEnabled->setChecked(mainWindow->getEffect()->getReflectionEnabled());
    ui->sliderReflectionOpacity->setValue(mainWindow->getEffect()->getReflectionOpacity());
    ui->sliderStarfieldAmount->setValue(mainWindow->getEffect()->getNumberOfStars());
    ui->sliderStarfieldSpeed->setValue(mainWindow->getEffect()->getStarSpeed());
    ui->checkBoxStarsEnabled->setChecked(mainWindow->getEffect()->getStarsEnabled());

    ui->checkBoxRotatingObjectOrbit->setChecked(mainWindow->getEffect()->getRotatingObjectOrbit());
    ui->checkBoxRotatingObjectWireframeEnabled->
            setChecked(mainWindow->getEffect()->getRotatingObjectWireframeEnabled());
    ui->comboBoxRotatingObjectModel->installEventFilter(this);
    ui->comboBoxRotatingObjectModel->addItem("Cube", "cube");
    ui->comboBoxRotatingObjectModel->addItem("Sphere", "sphere");
    ui->comboBoxRotatingObjectModel->addItem("Pyramid", "pyramid");
    const int dirindex = ui->comboBoxRotatingObjectModel->findData(mainWindow->getEffect()->getRotatingObjectModel());
    ui->comboBoxRotatingObjectModel->setCurrentIndex(dirindex);

    ui->comboBoxRotatingObjectMaterial->installEventFilter(this);
    ui->comboBoxRotatingObjectMaterial->addItem("None", "none");
    ui->comboBoxRotatingObjectMaterial->addItem("Flat", "flat");
    ui->comboBoxRotatingObjectMaterial->addItem("Lambert", "lambert");
    ui->comboBoxRotatingObjectMaterial->addItem("Blinn", "blinn");
    const int materialindex = ui->comboBoxRotatingObjectMaterial->findData(
        mainWindow->getEffect()->getRotatingObjectMaterial());
    ui->comboBoxRotatingObjectMaterial->setCurrentIndex(materialindex);

    ui->sliderRotatingObjectFocalLength->setValue(mainWindow->getEffect()->getRotatingObjectFocalLength());
    ui->sliderRotatingObjectModelSize->setValue(mainWindow->getEffect()->getRotatingObjectSize());
    ui->sliderRotatingObjectOrbitSize->setValue(mainWindow->getEffect()->getRotatingObjectOrbitSize());
    ui->sliderRotatingObjectOrbitSpeed->setValue(mainWindow->getEffect()->getRotatingObjectOrbitSpeed());
    ui->checkBoxRotatingObjectEnabled->setChecked(mainWindow->getEffect()->getRotatingObjectEnabled());
    // don't understand why it's not triggered, hence the following row
    on_checkBoxRotatingObjectEnabled_toggled(mainWindow->getEffect()->getRotatingObjectEnabled());

    ui->textEditScrollerCustomText->setText(mainWindow->getEffect()->getCustomScrolltext());
    ui->checkBoxScrollerCustomTextEnabled->setChecked(mainWindow->getEffect()->getCustomScrolltextEnabled());
    ui->checkBoxScrollerEnabled->setChecked(mainWindow->getEffect()->getScrollerEnabled());

    on_checkBoxScrollerEnabled_toggled(mainWindow->getEffect()->getScrollerEnabled());

    ui->checkBoxPrinterEnabled->setChecked(mainWindow->getEffect()->getPrinterEnabled());
    ui->checkBoxVuMeterEnabled->setChecked(mainWindow->getEffect()->isVuMeterEnabled());
    on_checkBoxVuMeterEnabled_toggled(mainWindow->getEffect()->isVuMeterEnabled());

    ui->checkBoxScrollerSinusFontScaling->setChecked(mainWindow->getEffect()->getSinusFontScalingEnabled());
    ui->comboBoxStarfieldDirection->installEventFilter(this);
    ui->comboBoxStarfieldDirection->addItem("Left", "left");
    ui->comboBoxStarfieldDirection->addItem("Right", "right");
    ui->comboBoxStarfieldDirection->addItem("Up", "up");
    ui->comboBoxStarfieldDirection->addItem("Down", "down");
    ui->comboBoxStarfieldDirection->addItem("In", "in");
    ui->comboBoxStarfieldDirection->addItem("Out", "out");
    const int modelindex = ui->comboBoxStarfieldDirection->findData(mainWindow->getEffect()->getStarsDirection());
    ui->comboBoxStarfieldDirection->setCurrentIndex(modelindex);
    ui->sliderVisualizerResolutionWidth->setValue(mainWindow->getEffect()->getResolutionWidth());
    ui->sliderVisualizerResolutionHeight->setValue(mainWindow->getEffect()->getResolutionHeight());
    ui->checkBoxVisualizerMaintainAspectRatio->setChecked(mainWindow->getEffect()->getKeepAspectRatio());
    ui->checkBoxRasterBarsEnabled->setChecked(mainWindow->getEffect()->getRasterBarsEnabled());
    ui->sliderRasterBarsAmount->setValue(mainWindow->getEffect()->getNumberOfRasterBars());
    ui->sliderRasterBarsHeight->setValue(mainWindow->getEffect()->getRasterBarsHeight());
    ui->sliderRasterBarsVerticalSpacing->setValue(mainWindow->getEffect()->getRasterBarsVerticalSpacing());
    ui->sliderRasterBarsSpeed->setValue(mainWindow->getEffect()->getRasterBarsSpeed());
    ui->sliderRasterBarsOpacity->setValue(mainWindow->getEffect()->getRasterbarsOpacity());
    forceUpdateToSliders();

    if (PLUGIN_libsidplayfp_LIB != "") {
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->installEventFilter(this);
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->addItem("At every start", "At every start");
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->addItem("Daily", "Daily");
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->addItem("Weekly", "Weekly");
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->addItem("Monthly", "Monthly");
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->addItem("Never", "Never");

        index = ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->findData(
            mainWindow->getBundledHvscSonglengthsUpdateFrequency());
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->setCurrentIndex(index);
    }

    int extensionPos = mainWindow->getEffect()->getFont().lastIndexOf('.');
    const QString thumb(mainWindow->getEffect()->getFont().left(extensionPos) + ".thumb.png");
    ui->buttonScrollerFontImage->setIcon(QIcon(thumb));
    extensionPos = mainWindow->getEffect()->getPrinterFont().lastIndexOf('.');
    const QString thumbP(mainWindow->getEffect()->getPrinterFont().left(extensionPos) + ".thumb.png");
    ui->buttonPrinterFontImage->setIcon(QIcon(thumbP));

    loadSettingsFmod();

    if (PLUGIN_adplug_LIB != "") {
        loadSettingsAdplug();
    }

    if (PLUGIN_furnace_LIB != "") {
        loadSettingsFurnace();
    }

    if (PLUGIN_highly_experimental_LIB != "") {
        loadSettingsHighlyExperimental();
    }

    if (PLUGIN_highly_quixotic_LIB != "") {
        loadSettingsHighlyQuixotic();
    }

    if (PLUGIN_hivelytracker_LIB != "") {
        loadSettingsHivelytracker();
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        loadSettingsLibsidplayfp();
    }

    if (PLUGIN_libvgm_LIB != "") {
        loadSettingsLibvgm();
    }

    if (PLUGIN_libxmp_LIB != "") {
        loadSettingsLibxmp();
    }

    if (PLUGIN_sndh_player_LIB != "") {
        loadSettingsSndhPlayer();
    }

    if (PLUGIN_vgmstream_LIB != "") {
        loadSettingsVgmstream();
    }

    loadSettingsLibopenmpt();
    loadSettingsUade();

    updateColorButtons();

    const QFont roboto("Roboto");
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

    if (PLUGIN_adplug_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_adplug_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_adplug_NAME));
    }

    if (PLUGIN_asap_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_asap_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_asap_NAME));
    }

    if (PLUGIN_audiodecoder_wsr_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_audiodecoder_wsr_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_audiodecoder_wsr_NAME));
    }

    if (PLUGIN_audiofile_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_audiofile_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_audiofile_NAME));
    }

    if (PLUGIN_flod_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_flod_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_flod_NAME));
    }

    if (PLUGIN_furnace_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_furnace_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_furnace_NAME));
    }

    if (PLUGIN_game_music_emu_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_game_music_emu_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_game_music_emu_NAME));
    }

    if (PLUGIN_highly_experimental_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_experimental_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_experimental_NAME));
    }

    if (PLUGIN_highly_quixotic_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_quixotic_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_quixotic_NAME));
    }

    if (PLUGIN_highly_theoretical_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_highly_theoretical_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_highly_theoretical_NAME));
    }

    if (PLUGIN_hivelytracker_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_hivelytracker_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_hivelytracker_NAME));
    }

    if (PLUGIN_jaytrax_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_jaytrax_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_jaytrax_NAME));
    }

    if (PLUGIN_kdm_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_kdm_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_kdm_NAME));
    }

    if (PLUGIN_klystron_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_klystron_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_klystron_NAME));
    }

    if (PLUGIN_lazyusf2_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_lazyusf2_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_lazyusf2_NAME));
    }

    if (PLUGIN_libkss_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libkss_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libkss_NAME));
    }

    if (PLUGIN_libopenmpt_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libopenmpt_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libopenmpt_NAME));
    }

    if (PLUGIN_libpac_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libpac_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libpac_NAME));
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libsidplayfp_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libsidplayfp_NAME));
    }

    if (PLUGIN_libstsound_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libstsound_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libstsound_NAME));
    }

    if (PLUGIN_libxmp_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libxmp_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libxmp_NAME));
    }

    if (PLUGIN_mdxmini_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_mdxmini_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_mdxmini_NAME));
    }

    if (PLUGIN_organya_decoder_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_organya_decoder_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_organya_decoder_NAME));
    }

    if (PLUGIN_protrekkr_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_protrekkr_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_protrekkr_NAME));
    }

    if (PLUGIN_sc68_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sc68_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sc68_NAME));
    }

    if (PLUGIN_sndh_player_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sndh_player_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sndh_player_NAME));
    }

    if (PLUGIN_sunvox_lib_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_sunvox_lib_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_sunvox_lib_NAME));
    }

    if (PLUGIN_v2m_player_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_v2m_player_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_v2m_player_NAME));
    }

    if (PLUGIN_libvgm_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_libvgm_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_libvgm_NAME));
    }

    if (PLUGIN_vgmstream_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_vgmstream_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_vgmstream_NAME));
    }

    if (PLUGIN_vio2sf_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_vio2sf_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_vio2sf_NAME));
    }

    if (PLUGIN_uade_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_uade_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_uade_NAME));
    }

    if (PLUGIN_zxtune_LIB != "") {
        ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(PLUGIN_zxtune_VERSION));
        ui->tableWidgetPlugins->setItem(row++, 0, new QTableWidgetItem(PLUGIN_zxtune_NAME));
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        const QDateTime qdt = QDateTime::fromSecsSinceEpoch(mainWindow->getBundledHvscSonglengthsDownloadEpoch());
        if (mainWindow->getBundledHvscSonglengthsDownloadEpoch() > 0) {
            ui->labelLibsidplayfpHvscSonglengthsDownloadDetails->setText(
                "Downloaded to " + mainWindow->getBundledHvscSonglengthsPath() + " at " + qdt.toString(
                    "yyyy-MM-dd hh:mm:ss"));
        } else {
            ui->labelLibsidplayfpHvscSonglengthsDownloadDetails->setText("Never downloaded");
        }
    }

    ui->tableWidgetPlugins->sortItems(0);
    ui->tableWidgetPlugins->setRowCount(row);
    ui->albumGridScrollerFont->setVisible(false);
    ui->albumGridPrinterFont->setVisible(false);

    const QDir directory(dataPath + RESOURCES_DIR + "/visualizer/bitmapfonts");
    QStringList images = directory.entryList(QStringList() << "*.thumb.png", QDir::Files);

    foreach(QString filename, images) {
        QString fullfilename = dataPath + RESOURCES_DIR + "/visualizer/bitmapfonts/" +
                               filename;
        const auto album = new Album(filename);
        album->artwork = fullfilename;
        album->title = filename.replace(".thumb.png", "");
        album->path = fullfilename.replace(".thumb.png", ".png");
        album->putPixmap(album->artwork);
        ui->albumGridScrollerFont->AddAlbum(album);

        const auto albumPrinter = new Album(filename);
        albumPrinter->artwork = fullfilename;
        albumPrinter->title = filename.replace(".thumb.png", "");
        albumPrinter->path = fullfilename.replace(".thumb.png", ".png");
        albumPrinter->putPixmap(album->artwork);
        ui->albumGridPrinterFont->AddAlbum(albumPrinter);

        connect(album, SIGNAL(clickedAddAlbum(QString)), this, SLOT(loadBitmapFont(const QString &)));
        connect(albumPrinter, SIGNAL(clickedAddAlbum(QString)), this, SLOT(loadBitmapFontPrinter(const QString &)));
    }

    updateCheckBoxes();
    on_buttonGeneral_clicked();
    changeStyleSheetColor();
}

settingsWindow::~settingsWindow() {
    delete ui;
}

void settingsWindow::on_sliderDefaultAudioLevel_sliderMoved(int position) {
}

void settingsWindow::on_checkBoxDefaultAudioLevel_toggled(const bool isChecked) const {
    ui->labelDefaultAudioLevelValue->setEnabled(isChecked);
    ui->sliderDefaultAudioLevel->setEnabled(isChecked);
    ui->checkBoxDefaultAudioLevel->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_sliderDefaultAudioLevel_valueChanged(const int value) const {
    ui->labelDefaultAudioLevelValue->setText(QString::number(value) + "%");
}

void settingsWindow::on_sliderNormalizerFadeTime_valueChanged(const int value) const {
    ui->labelNormalizerFadeTimeValue->setText(QString::number(value));
    mainWindow->setNormalizeFadeTime(ui->sliderNormalizerFadeTime->value());
}

void settingsWindow::on_sliderNormalizerThreshold_valueChanged(const int value) const {
    ui->labelNormalizerThresholdValue->setText(QString::number(value) + "%");
    mainWindow->setNormalizeThreshold(ui->sliderNormalizerThreshold->value());
}

void settingsWindow::on_sliderNormalizerMaxAmp_valueChanged(const int value) const {
    ui->labelNormalizerMaxAmpValue->setText(QString::number(value));
    mainWindow->setNormalizeMaxAmp(ui->sliderNormalizerMaxAmp->value());
}

void settingsWindow::on_checkBoxNormalizer_toggled(const bool isChecked) const {
    mainWindow->addDebugText("on_checkBoxNormalizer_toggled");
    ui->sliderNormalizerFadeTime->setEnabled(isChecked);
    ui->sliderNormalizerThreshold->setEnabled(isChecked);
    ui->sliderNormalizerMaxAmp->setEnabled(isChecked);
    ui->labelNormalizerFadeTimeValue->setEnabled(isChecked);
    ui->labelNormalizerThresholdValue->setEnabled(isChecked);
    ui->labelNormalizerMaxAmpValue->setEnabled(isChecked);

    mainWindow->setNormalizeEnabled(isChecked);
    ui->checkBoxNormalizer->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxReverb_toggled(const bool isChecked) const {
    mainWindow->addDebugText("on_checkBoxReverb_toggled");
    ui->comboBoxReverb->setEnabled(isChecked);

    mainWindow->setReverbEnabled(isChecked);
    ui->checkBoxReverb->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

bool settingsWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Wheel &&
        (obj == ui->sliderVuMeterWidth ||
         obj == ui->sliderVuMeterOpacity ||
         obj == ui->sliderScrollerAmplitude ||
         obj == ui->sliderScrollerFrequency ||
         obj == ui->sliderScrollerSinusSpeed ||
         obj == ui->sliderScrollerScrollSpeed ||
         obj == ui->sliderScrollerVerticalPosition ||
         obj == ui->sliderScrollerFontXScale ||
         obj == ui->sliderScrollerFontYScale ||
         obj == ui->sliderPrinterFontXScale ||
         obj == ui->sliderPrinterFontYScale ||
         obj == ui->sliderVuMeterPeakHeight ||
         obj == ui->sliderReflectionOpacity ||
         obj == ui->sliderStarfieldAmount ||
         obj == ui->sliderStarfieldSpeed ||
         obj == ui->sliderAppearancePlaylistRowHeight ||
         obj == ui->sliderAppearancePlaylistItemRowHeight ||
         obj == ui->sliderAppearanceNowPlayingFontSize ||
         obj == ui->sliderDefaultAudioLevel ||
         obj == ui->sliderNormalizerFadeTime ||
         obj == ui->sliderNormalizerMaxAmp ||
         obj == ui->sliderNormalizerThreshold ||
         obj == ui->comboBoxReverb ||
         obj == ui->comboBoxUadeFreq ||
         obj == ui->comboBoxUadeResampler ||
         obj == ui->sliderUadePanning ||
         obj == ui->comboBoxUadeFilterEmuMode ||
         obj == ui->comboBoxUadeLedFilter ||
         obj == ui->comboBoxOutputDevice ||
         obj == ui->comboBoxStarfieldDirection ||
         obj == ui->sliderVisualizerResolutionHeight ||
         obj == ui->sliderVisualizerResolutionWidth ||
         obj == ui->sliderRasterBarsHeight ||
         obj == ui->sliderRasterBarsSpeed ||
         obj == ui->sliderRasterBarsOpacity ||
         obj == ui->sliderRasterBarsVerticalSpacing ||
         obj == ui->sliderRasterBarsAmount ||
         obj == ui->sliderRotatingObjectFocalLength ||
         obj == ui->sliderRotatingObjectModelSize ||
         obj == ui->comboBoxLibopenmptFilter ||
         obj == ui->comboBoxLibopenmptResampling ||
         obj == ui->comboBoxLibopenmptDither ||
         obj == ui->sliderLibopenmptStereoSeparation ||
         obj == ui->comboBoxLibsidplayfpHvscSonglengthsUpdate ||
         obj == ui->sliderUadeSilenceTimeOut ||
         obj == ui->comboBoxAdPlugEmulator ||
         obj == ui->comboBoxAdPlugFreq ||
         obj == ui->comboBoxAdPlugPlayback ||
         obj == ui->comboBoxHivelyTrackerStereoSeparation ||
         obj == ui->comboBoxDefaultPlayMode ||
         obj == ui->comboBoxRotatingObjectMaterial ||
         obj == ui->comboBoxRotatingObjectModel ||
         obj == ui->sliderRotatingObjectOrbitSize ||
         obj == ui->sliderRotatingObjectOrbitSpeed ||
         obj == ui->sliderDefaultTrackLength)) {
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void settingsWindow::on_buttonOK_clicked() {
    if (ui->comboBoxOutputDevice->itemData(ui->comboBoxOutputDevice->currentIndex()).toInt() !=
        mainWindow->getOutputDevice()) {
        mainWindow->setOutputDeviceSetting(
            ui->comboBoxOutputDevice->itemData(ui->comboBoxOutputDevice->currentIndex()).toInt());
    }

    mainWindow->setResetVolume(ui->checkBoxDefaultAudioLevel->checkState() == Qt::Checked);
    mainWindow->setResetVolumeValue(ui->sliderDefaultAudioLevel->value());
    mainWindow->setDefaultPlayMode(ui->comboBoxDefaultPlayMode->currentData().toInt());
    mainWindow->setSystrayChecked(ui->checkBoxSystray->checkState() == Qt::Checked);
    mainWindow->setSystrayOnMinimizeChecked(ui->checkBoxMinimizeToSystray->checkState() == Qt::Checked);
    mainWindow->setMenuBarHiddenChecked(ui->checkBoxMenuBarHidden->checkState() == Qt::Checked);
    mainWindow->setIgnoreSuffix(ui->lineEditIgnoreSuffixes->text());
    mainWindow->setIgnorePrefix(ui->lineEditIgnorePrefixes->text());
    mainWindow->setDefaultTrackLengthEnable(ui->checkBoxDefaultTrackLength->checkState() == Qt::Checked);
    mainWindow->setDefaultTrackLengthValue(ui->sliderDefaultTrackLength->value());

    updateScrollText();

    saveSettingsFmod();

    if (PLUGIN_adplug_LIB != "") {
        saveSettingsAdplug();
    }

    if (PLUGIN_furnace_LIB != "") {
        saveSettingsFurnace();
    }

    if (PLUGIN_highly_experimental_LIB != "") {
        saveSettingsHighlyExperimental();
    }

    if (PLUGIN_highly_quixotic_LIB != "") {
        saveSettingsHighlyQuixotic();
    }

    if (PLUGIN_hivelytracker_LIB != "") {
        saveSettingsHivelytracker();
    }

    if (PLUGIN_libsidplayfp_LIB != "") {
        saveSettingsLibsidplayfp();
    }

    if (PLUGIN_libvgm_LIB != "") {
        saveSettingsLibvgm();
    }

    if (PLUGIN_libxmp_LIB != "") {
        saveSettingsLibxmp();
    }

    if (PLUGIN_sndh_player_LIB != "") {
        saveSettingsSndhPlayer();
    }

    if (PLUGIN_vgmstream_LIB != "") {
        saveSettingsVgmstream();
    }

    saveSettingsLibopenmpt();
    saveSettingsUade();

    mainWindow->saveSettings();

    close();
}

void settingsWindow::on_comboBoxReverb_textActivated(const QString &arg1) const {
    mainWindow->setReverbPreset(arg1);
    const bool checkedReverb = ui->checkBoxReverb->checkState() == Qt::Checked;
    mainWindow->setReverbEnabled(checkedReverb);
}

void settingsWindow::on_buttonLibsidplayfpHvscSonglengthsBrowse_clicked() {
    const QString startFolder = ui->lineEditLibsidplayfpHvscSonglengthsPath->text();

    if (const QString file = QFileDialog::getOpenFileName(this, "Choose your Songlengths.md5", startFolder, "*.md5");
        !file.isEmpty()) {
        setUiLineEditLibsidplayfpHvscSonglengthsPath(file);
        mainWindow->setHvscSonglengthsPath(file);
    }
}

void settingsWindow::loadSettingsAdplug() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/adplug.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->comboBoxAdPlugEmulator->setCurrentIndex(ui->comboBoxAdPlugEmulator->findData("2"));
    ui->comboBoxAdPlugFreq->setCurrentIndex(ui->comboBoxAdPlugFreq->findData("44100"));
    ui->comboBoxAdPlugPlayback->setCurrentIndex(ui->comboBoxAdPlugPlayback->findData("2"));
    ui->checkBoxAdPlugContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("emulator") == 0) {
                    int index = ui->comboBoxAdPlugEmulator->findData(value.c_str());
                    ui->comboBoxAdPlugEmulator->setCurrentIndex(index);
                } else if (word.compare("frequency") == 0) {
                    int index = ui->comboBoxAdPlugFreq->findData(value.c_str());
                    ui->comboBoxAdPlugFreq->setCurrentIndex(index);
                } else if (word.compare("playback") == 0) {
                    int index = ui->comboBoxAdPlugPlayback->findData(value.c_str());
                    ui->comboBoxAdPlugPlayback->setCurrentIndex(index);
                } else if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxAdPlugContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsFmod() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/fmod.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxFmodSeamlessLoop->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("seamlessLoop") == 0) {
                    ui->checkBoxFmodSeamlessLoop->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }

    mainWindow->setFmodSeamlessLoopEnabled(ui->checkBoxFmodSeamlessLoop->isChecked());
}

void settingsWindow::loadSettingsFurnace() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/furnace.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxFurnaceContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxFurnaceContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsHighlyExperimental() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/highly_experimental.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxHighlyExperimentalContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxHighlyExperimentalContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsHighlyQuixotic() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/highly_quixotic.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxHighlyQuixoticContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxHighlyQuixoticContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsHivelytracker() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/hivelytracker.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    int index = ui->comboBoxHivelyTrackerStereoSeparation->findData("2");
    ui->comboBoxHivelyTrackerStereoSeparation->setCurrentIndex(index);
    ui->checkBoxHivelyTrackerContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("stereoSeparation") == 0) {
                    int index = ui->comboBoxHivelyTrackerStereoSeparation->findData(value.c_str());
                    ui->comboBoxHivelyTrackerStereoSeparation->setCurrentIndex(index);
                } else if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxHivelyTrackerContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsLibopenmpt() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libopenmpt.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->sliderLibopenmptStereoSeparation->setValue(100);
    ui->checkBoxLibopenmptContinuousPlayback->setChecked(false);
    ui->checkBoxLibopenmptAmigaResampler->setChecked(true);
    int index = ui->comboBoxLibopenmptFilter->findData("auto");
    ui->comboBoxLibopenmptFilter->setCurrentIndex(index);
    int index2 = ui->comboBoxLibopenmptResampling->findData("0");
    ui->comboBoxLibopenmptResampling->setCurrentIndex(index2);
    int index3 = ui->comboBoxLibopenmptDither->findData("1");
    ui->comboBoxLibopenmptDither->setCurrentIndex(index3);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("stereoSeparation") == 0) {
                    ui->sliderLibopenmptStereoSeparation->setValue(atoi(value.c_str()));
                } else if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxLibopenmptContinuousPlayback->setChecked(value.compare("true") == 0);
                } else if (word.compare("emulateAmigaFilter") == 0) {
                    if (value.compare("true") == 0) {
                        ui->checkBoxLibopenmptAmigaResampler->setChecked(true);
                    } else {
                        ui->checkBoxLibopenmptAmigaResampler->setChecked(false);
                    }
                } else if (word.compare("interpolationFilter") == 0) {
                    int index = ui->comboBoxLibopenmptResampling->findData(value.c_str());
                    ui->comboBoxLibopenmptResampling->setCurrentIndex(index);
                } else if (word.compare("amigaFilter") == 0) {
                    int index = ui->comboBoxLibopenmptFilter->findData(value.c_str());
                    ui->comboBoxLibopenmptFilter->setCurrentIndex(index);
                } else if (word.compare("dither") == 0) {
                    int index = ui->comboBoxLibopenmptDither->findData(value.c_str());
                    ui->comboBoxLibopenmptDither->setCurrentIndex(index);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsLibsidplayfp() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libsidplayfp.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // hack, because "toggled" means changing value,
    // so just to be sure, set it to both values so
    // that on_checkBoxLibsidplayfpHvscSonglengthsEnabled_toggled is called
    ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(false);
    ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(true);

    ui->checkBoxLibsidplayfpContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("hvscSonglengthsPath") == 0) {
                    QString hvscSonglengthsPathToLoad = value.c_str();

                    if (hvscSonglengthsPathToLoad.isEmpty()) {
                        hvscSonglengthsPathToLoad = mainWindow->getBundledHvscSonglengthsPath();
                    }

                    mainWindow->setHvscSonglengthsPath(hvscSonglengthsPathToLoad);
                    setUiLineEditLibsidplayfpHvscSonglengthsPath(hvscSonglengthsPathToLoad);
                } else if (word.compare("hvscSonglengthsEnabled") == 0) {
                    if (value.compare("true") == 0) {
                        ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(true);
                    } else {
                        ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(false);
                    }
                } else if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxLibsidplayfpContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    } else {
        ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(false);
        ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setChecked(true);
        mainWindow->setHvscSonglengthsPath(mainWindow->getBundledHvscSonglengthsPath());
        setUiLineEditLibsidplayfpHvscSonglengthsPath(mainWindow->getBundledHvscSonglengthsPath());
    }
}

void settingsWindow::loadSettingsLibvgm() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libvgm.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxLibvgmContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxLibvgmContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsLibxmp() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libxmp.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxLibxmpContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxLibxmpContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsSndhPlayer() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/sndh-player.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxSndhPlayerContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxSndhPlayerContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsUade() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/uade.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->comboBoxUadeFreq->setCurrentIndex(ui->comboBoxUadeFreq->findData("48000"));
    ui->comboBoxUadeResampler->setCurrentIndex(ui->comboBoxUadeResampler->findData("sinc"));
    ui->checkBoxUadeFilterEmu->setChecked(true);
    ui->comboBoxUadeFilterEmuMode->setCurrentIndex(1);
    ui->comboBoxUadeLedFilter->setCurrentIndex(0);
    ui->sliderUadePanning->setValue(5);
    ui->labelUadePanningValue->setText("0.5");
    ui->sliderUadeSilenceTimeOut->setValue(5);
    ui->lineEditUadeSonglengthsPath->setText("/uade.md5");
    ui->checkBoxUadeSongLengths->setChecked(true);
    ui->checkBoxUadeContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("frequency") == 0) {
                    int index = ui->comboBoxUadeFreq->findData(value.c_str());
                    ui->comboBoxUadeFreq->setCurrentIndex(index);
                } else if (word.compare("resampler") == 0) {
                    int index = ui->comboBoxUadeResampler->findData(value.c_str());
                    ui->comboBoxUadeResampler->setCurrentIndex(index);
                } else if (word.compare("filterEmu") == 0) {
                    if (value.compare("true") == 0) {
                        ui->checkBoxUadeFilterEmu->setChecked(true);
                    } else {
                        ui->checkBoxUadeFilterEmu->setChecked(false);
                    }
                } else if (word.compare("filterMode") == 0) {
                    if (value.compare("a500") == 0) {
                        ui->comboBoxUadeFilterEmuMode->setCurrentIndex(0);
                    } else {
                        ui->comboBoxUadeFilterEmuMode->setCurrentIndex(1);
                    }
                } else if (word.compare("ledForced") == 0) {
                    if (value.compare("auto") == 0) {
                        ui->comboBoxUadeLedFilter->setCurrentIndex(0);
                    } else if (value.compare("on") == 0) {
                        ui->comboBoxUadeLedFilter->setCurrentIndex(1);
                    } else {
                        ui->comboBoxUadeLedFilter->setCurrentIndex(2);
                    }
                } else if (word.compare("panning") == 0) {
                    ui->sliderUadePanning->setValue(stoi(value));
                } else if (word.compare("silenceTimeoutEnabled") == 0) {
                    if (value.compare("true") == 0) {
                        ui->checkBoxUadeSilenceTimeout->setChecked(true);
                    } else {
                        ui->checkBoxUadeSilenceTimeout->setChecked(false);
                    }
                } else if (word.compare("silenceTimeout") == 0) {
                    ui->sliderUadeSilenceTimeOut->setValue(atoi(value.c_str()));
                } else if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxUadeContinuousPlayback->setChecked(value.compare("true") == 0);
                } else if (word.compare("uadeSonglengthsEnabled") == 0) {
                    if (value.compare("true") == 0) {
                        ui->checkBoxUadeSongLengths->setChecked(true);
                    } else {
                        ui->checkBoxUadeSongLengths->setChecked(false);
                    }
                } else if (word.compare("uadeSonglengthsPath") == 0) {
                    if (value == "") {
                        ui->lineEditUadeSonglengthsPath->setText("/uade.md5");
                    } else {
                        ui->lineEditUadeSonglengthsPath->setText(value.c_str());
                    }
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::loadSettingsVgmstream() const {
    // read config from disk
    string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/vgmstream.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    ui->checkBoxVgmstreamContinuousPlayback->setChecked(false);

    if (!useDefaults) {
        string line;

        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);

                if (word.compare("continuousPlayback") == 0) {
                    ui->checkBoxVgmstreamContinuousPlayback->setChecked(value.compare("true") == 0);
                }
            }
        }

        ifs.close();
    }
}

void settingsWindow::saveSettingsAdplug() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/adplug.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "emulator=" << ui->comboBoxAdPlugEmulator->currentData().toString().toStdString().c_str() << "\n";
    ofs << "frequency=" << ui->comboBoxAdPlugFreq->currentData().toString().toStdString().c_str() << "\n";
    ofs << "playback=" << ui->comboBoxAdPlugPlayback->currentData().toString().toStdString().c_str() << "\n";
    ofs << "continuousPlayback=" << (ui->checkBoxAdPlugContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsFmod() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/fmod.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    const bool isFmodSeamlessLoopEnabled = ui->checkBoxFmodSeamlessLoop->isChecked();

    ofs << "seamlessLoop=" << (isFmodSeamlessLoopEnabled ? "true" : "false") << "\n";
    ofs.close();

    mainWindow->setFmodSeamlessLoopEnabled(isFmodSeamlessLoopEnabled);
}

void settingsWindow::saveSettingsFurnace() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/furnace.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxFurnaceContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsHighlyExperimental() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/highly_experimental.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxHighlyExperimentalContinuousPlayback->isChecked() ? "true" : "false")
            << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsHighlyQuixotic() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/highly_quixotic.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxHighlyQuixoticContinuousPlayback->isChecked() ? "true" : "false")
            << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsHivelytracker() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/hivelytracker.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "stereoSeparation=" << ui->comboBoxHivelyTrackerStereoSeparation->currentData().toString().toStdString().
            c_str() << "\n";
    ofs << "continuousPlayback=" << (ui->checkBoxHivelyTrackerContinuousPlayback->isChecked() ? "true" : "false") <<
            "\n";
    ofs.close();
}

void settingsWindow::saveSettingsLibopenmpt() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libopenmpt.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "stereoSeparation=" << ui->sliderLibopenmptStereoSeparation->value() << "\n";
    ofs << "continuousPlayback=" << (ui->checkBoxLibopenmptContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs << "interpolationFilter=" << ui->comboBoxLibopenmptResampling->currentData().toString().toStdString().c_str() <<
            "\n";
    ofs << "amigaFilter=" << ui->comboBoxLibopenmptFilter->currentData().toString().toStdString().c_str() << "\n";
    ofs << "dither=" << ui->comboBoxLibopenmptDither->currentData().toString().toStdString().c_str() << "\n";
    ofs << "emulateAmigaFilter=" << (ui->checkBoxLibopenmptAmigaResampler->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsLibsidplayfp() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libsidplayfp.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxLibsidplayfpContinuousPlayback->isChecked() ? "true" : "false") <<
            "\n";
    ofs << "hvscSonglengthsEnabled=" << (ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->isChecked() ? "true" : "false")
            << "\n";

    QString hvscSonglengthsPathToSave = ui->lineEditLibsidplayfpHvscSonglengthsPath->text();

    if (hvscSonglengthsPathToSave.isEmpty()) {
        hvscSonglengthsPathToSave = mainWindow->getBundledHvscSonglengthsPath();
    }

    ofs << "hvscSonglengthsPath=" << hvscSonglengthsPathToSave.toStdString().c_str() << "\n";

    mainWindow->setHvscSonglengthsPath(hvscSonglengthsPathToSave);
    ofs.close();
}

void settingsWindow::saveSettingsLibvgm() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libvgm.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxLibvgmContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsLibxmp() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/libxmp.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxLibxmpContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsSndhPlayer() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/sndh-player.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxSndhPlayerContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsUade() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/uade.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    QString filterMode;

    if (ui->comboBoxUadeFilterEmuMode->currentIndex() == 0) {
        filterMode = "a500";
    } else {
        filterMode = "a1200";
    }

    QString ledFilter;

    if (ui->comboBoxUadeLedFilter->currentIndex() == 0) {
        ledFilter = "auto";
    } else if (ui->comboBoxUadeLedFilter->currentIndex() == 1) {
        ledFilter = "on";
    } else {
        ledFilter = "off";
    }

    ofs << "frequency=" << ui->comboBoxUadeFreq->currentData().toString().toStdString().c_str() << "\n";
    ofs << "resampler=" << ui->comboBoxUadeResampler->currentData().toString().toStdString().c_str() << "\n";
    ofs << "filterEmu=" << (ui->checkBoxUadeFilterEmu->isChecked() ? "true" : "false") << "\n";
    ofs << "filterMode=" << filterMode.toStdString().c_str() << "\n";
    ofs << "ledForced=" << ledFilter.toStdString().c_str() << "\n";
    ofs << "panning=" << ui->sliderUadePanning->value() << "\n";
    ofs << "silenceTimeout=" << ui->sliderUadeSilenceTimeOut->value() << "\n";
    ofs << "silenceTimeoutEnabled=" << (ui->checkBoxUadeSilenceTimeout->isChecked() ? "true" : "false") << "\n";
    ofs << "continuousPlayback=" << (ui->checkBoxUadeContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs << "uadeSonglengthsEnabled=" << (ui->checkBoxUadeSongLengths->isChecked() ? "true" : "false") << "\n";
    ofs << "uadeSonglengthsPath=" << ui->lineEditUadeSonglengthsPath->text().toStdString().c_str() << "\n";
    ofs.close();
}

void settingsWindow::saveSettingsVgmstream() const {
    // save config to disk
    const string filename = userPath.toStdString() + PLUGINS_CONFIG_DIR + "/vgmstream.cfg";
    ofstream ofs(filename.c_str());

    if (ofs.fail()) {
        // the file could not be opened
        return;
    }

    ofs << "continuousPlayback=" << (ui->checkBoxVgmstreamContinuousPlayback->isChecked() ? "true" : "false") << "\n";
    ofs.close();
}

void settingsWindow::on_tableWidgetPlugins_itemClicked(QTableWidgetItem *item) const {
    if (const int row = item->row(); ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_adplug_NAME) {
        ui->groupBoxAdplug->setHidden(false);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_fmod_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(false);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_furnace_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(false);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_highly_experimental_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(false);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_highly_quixotic_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(false);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_hivelytracker_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(false);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libopenmpt_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(false);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libsidplayfp_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(false);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libvgm_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(false);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_libxmp_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(false);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_sndh_player_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(false);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_uade_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(false);
        ui->groupBoxVgmstream->setHidden(true);
    } else if (ui->tableWidgetPlugins->item(row, 0)->text() == PLUGIN_vgmstream_NAME) {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(false);
    } else {
        ui->groupBoxAdplug->setHidden(true);
        ui->groupBoxFmod->setHidden(true);
        ui->groupBoxFurnace->setHidden(true);
        ui->groupBoxHighlyExperimental->setHidden(true);
        ui->groupBoxHighlyQuixotic->setHidden(true);
        ui->groupBoxHivelytracker->setHidden(true);
        ui->groupBoxLibopenmpt->setHidden(true);
        ui->groupBoxLibsidplayfp->setHidden(true);
        ui->groupBoxLibvgm->setHidden(true);
        ui->groupBoxLibxmp->setHidden(true);
        ui->groupBoxSndhPlayer->setHidden(true);
        ui->groupBoxUade->setHidden(true);
        ui->groupBoxVgmstream->setHidden(true);
    }
}

void settingsWindow::changeStyleSheetColor() {
    QString stylesheet = this->styleSheet();

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

    stylesheet = ui->buttonAppearanceResetColors->styleSheet();
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    ui->buttonAppearanceResetColors->setStyleSheet(stylesheet);

    stylesheet = ui->scrollAreaWidgetContentsGeneral->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContentsGeneral->setStyleSheet(stylesheet);

    stylesheet = ui->scrollAreaWidgetContentsAppearance->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContentsAppearance->setStyleSheet(stylesheet);

    stylesheet = ui->scrollAreaWidgetContentsVisualizer->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->scrollAreaWidgetContentsVisualizer->setStyleSheet(stylesheet);

    stylesheet = ui->tableWidgetPlugins->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    ui->tableWidgetPlugins->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxAdplug->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxAdplug->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxFmod->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxFmod->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxFurnace->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxFurnace->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxHighlyExperimental->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxHighlyExperimental->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxHighlyQuixotic->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxHighlyQuixotic->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxHivelytracker->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxHivelytracker->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxUade->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxUade->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibsidplayfp->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibsidplayfp->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibopenmpt->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibopenmpt->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibvgm->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibvgm->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxLibxmp->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxLibxmp->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxSndhPlayer->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxSndhPlayer->setStyleSheet(stylesheet);

    stylesheet = ui->groupBoxVgmstream->styleSheet();
    stylesheet.replace(mainWindow->colorSelectionOld, mainWindow->getColorSelection());
    stylesheet.replace(mainWindow->colorBackgroundOld, mainWindow->getColorBackground());
    stylesheet.replace(mainWindow->colorMainOld, mainWindow->getColorMain());
    stylesheet.replace(mainWindow->colorMainHoverOld, mainWindow->getColorMainHover());
    stylesheet.replace(mainWindow->colorMediumOld, mainWindow->getColorMedium());
    stylesheet.replace(mainWindow->colorMainTextOld, mainWindow->getColorMainText());
    stylesheet.replace(mainWindow->colorButtonOld, mainWindow->getColorButton());
    stylesheet.replace(mainWindow->colorButtonHoverOld, mainWindow->getColorButtonHover());
    ui->groupBoxVgmstream->setStyleSheet(stylesheet);
}

void settingsWindow::setUiLineEditLibsidplayfpHvscSonglengthsPath(const QString &text) const {
    ui->lineEditLibsidplayfpHvscSonglengthsPath->setText(text);
}

void settingsWindow::on_buttonAppearanceMainColor_clicked() {
    const QColor oldColor(mainWindow->getColorMain().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorMain(newColor.name());
        mainWindow->channels->updateChannelColors();

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceMainColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceMediumColor_clicked() {
    const QColor oldColor(mainWindow->getColorMedium().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorMedium(newColor.name());
        mainWindow->channels->updateChannelColors();

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceMediumColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceBackgroundColor_clicked() {
    const QColor oldColor(mainWindow->getColorBackground().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color"); newColor.isValid()) {
        mainWindow->setColorBackground(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceBackgroundColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceBehindBackgroundColor_clicked() {
    const QColor oldColor(mainWindow->getColorBehindBackground().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorBehindBackground(newColor.name());
        mainWindow->channels->updateChannelColors();

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceBehindBackgroundColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceMainTextColor_clicked() {
    const QColor oldColor(mainWindow->getColorMainText().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorMainText(newColor.name());
        mainWindow->channels->updateChannelColors();

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceMainTextColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceButtonColor_clicked() {
    const QColor oldColor(mainWindow->getColorButton().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorButton(newColor.name());

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceButtonColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceDimmedTextColor_clicked() {
    const QColor oldColor(mainWindow->getColorDimmedText().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorDimmedText(newColor.name());

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceDimmedTextColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceMainHoverColor_clicked() {
    const QColor oldColor(mainWindow->getColorMainHover().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorMainHover(newColor.name());

        const auto qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceMainHoverColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceButtonHoverColor_clicked() {
    const QColor oldColor(mainWindow->getColorButtonHover().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorButtonHover(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceButtonHoverColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonAppearanceResetColors_clicked() const {
    mainWindow->resetToDefaultColors();
    updateColorButtons();
}

void settingsWindow::on_buttonVisualizer_clicked() const {
    ui->scrollAreaGeneral->setHidden(true);
    ui->scrollAreaAppearance->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(false);
    ui->tableWidgetPlugins->setHidden(true);
    ui->groupBoxAdplug->setHidden(true);
    ui->groupBoxFmod->setHidden(true);
    ui->groupBoxFurnace->setHidden(true);
    ui->groupBoxHighlyExperimental->setHidden(true);
    ui->groupBoxHighlyQuixotic->setHidden(true);
    ui->groupBoxHivelytracker->setHidden(true);
    ui->groupBoxLibopenmpt->setHidden(true);
    ui->groupBoxLibsidplayfp->setHidden(true);
    ui->groupBoxLibvgm->setHidden(true);
    ui->groupBoxLibxmp->setHidden(true);
    ui->groupBoxSndhPlayer->setHidden(true);
    ui->groupBoxUade->setHidden(true);
    ui->groupBoxVgmstream->setHidden(true);
}

void settingsWindow::on_buttonGeneral_clicked() const {
    ui->scrollAreaGeneral->setHidden(false);
    ui->scrollAreaAppearance->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(true);
    ui->tableWidgetPlugins->setHidden(true);
    ui->groupBoxAdplug->setHidden(true);
    ui->groupBoxFmod->setHidden(true);
    ui->groupBoxFurnace->setHidden(true);
    ui->groupBoxHighlyExperimental->setHidden(true);
    ui->groupBoxHighlyQuixotic->setHidden(true);
    ui->groupBoxHivelytracker->setHidden(true);
    ui->groupBoxLibopenmpt->setHidden(true);
    ui->groupBoxLibsidplayfp->setHidden(true);
    ui->groupBoxLibvgm->setHidden(true);
    ui->groupBoxLibxmp->setHidden(true);
    ui->groupBoxSndhPlayer->setHidden(true);
    ui->groupBoxUade->setHidden(true);
    ui->groupBoxVgmstream->setHidden(true);
}

void settingsWindow::on_buttonPlugins_clicked() const {
    ui->scrollAreaGeneral->setHidden(true);
    ui->scrollAreaAppearance->setHidden(true);
    ui->scrollAreaVisualizer->setHidden(true);
    ui->tableWidgetPlugins->setHidden(false);
}

void settingsWindow::on_buttonAppearance_clicked() const {
    ui->scrollAreaGeneral->setHidden(true);
    ui->scrollAreaAppearance->setHidden(false);
    ui->scrollAreaVisualizer->setHidden(true);
    ui->tableWidgetPlugins->setHidden(true);
    ui->groupBoxAdplug->setHidden(true);
    ui->groupBoxFmod->setHidden(true);
    ui->groupBoxFurnace->setHidden(true);
    ui->groupBoxHighlyExperimental->setHidden(true);
    ui->groupBoxHighlyQuixotic->setHidden(true);
    ui->groupBoxHivelytracker->setHidden(true);
    ui->groupBoxLibopenmpt->setHidden(true);
    ui->groupBoxLibsidplayfp->setHidden(true);
    ui->groupBoxLibvgm->setHidden(true);
    ui->groupBoxLibxmp->setHidden(true);
    ui->groupBoxSndhPlayer->setHidden(true);
    ui->groupBoxUade->setHidden(true);
    ui->groupBoxVgmstream->setHidden(true);
}

void settingsWindow::on_buttonVuMeterTopColor_clicked() {
    const QColor oldColor(mainWindow->getColorVisualizerTop());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorVisualizerTop(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonVuMeterTopColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonVuMeterBottomColor_clicked() {
    const QColor oldColor(mainWindow->getColorVisualizerBottom());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorVisualizerBottom(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonVuMeterBottomColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonVuMeterMiddleColor_clicked() {
    const QColor oldColor(mainWindow->getColorVisualizerMiddle());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorVisualizerMiddle(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonVuMeterMiddleColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonVisualizerBackgroundColor_clicked() {
    const QColor oldColor(mainWindow->getColorVisualizerBackground());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorVisualizerBackground(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonVisualizerBackgroundColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_sliderAppearancePlaylistItemRowHeight_valueChanged(const int value) const {
    ui->labelAppearancePlaylistItemRowHeightValue->setText(QString::number(value) + "px");
    mainWindow->setPlaylistRowHeight(value);
}

void settingsWindow::on_sliderAppearancePlaylistRowHeight_valueChanged(const int value) const {
    ui->labelAppearancePlaylistRowHeightValue->setText(QString::number(value) + "px");
    mainWindow->setPlaylistsRowHeight(value);
}

void settingsWindow::updateColorButtons() const {
    QString qss = QString("background-color: %1").arg(mainWindow->getColorMain());
    ui->buttonAppearanceMainColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorBackground());
    ui->buttonAppearanceBackgroundColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorBehindBackground());
    ui->buttonAppearanceBehindBackgroundColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMainText());
    ui->buttonAppearanceMainTextColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorSelection());
    ui->buttonAppearanceSelectionColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorDimmedText());
    ui->buttonAppearanceDimmedTextColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMedium());
    ui->buttonAppearanceMediumColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorButton());
    ui->buttonAppearanceButtonColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorMainHover());
    ui->buttonAppearanceMainHoverColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorButtonHover());
    ui->buttonAppearanceButtonHoverColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerTop());
    ui->buttonVuMeterTopColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerBottom());
    ui->buttonVuMeterBottomColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerMiddle());
    ui->buttonVuMeterMiddleColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerPeakColor());
    ui->buttonVuMeterPeakColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getEffect()->getReflectionColor());
    ui->buttonReflectionColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getEffect()->getRotatingObjectColor());
    ui->buttonRotatingObjectMaterialColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getEffect()->getRotatingObjectColorWireframe());
    ui->buttonRotatingObjectWireframeColor->setStyleSheet(qss);

    qss = QString("background-color: %1").arg(mainWindow->getColorVisualizerBackground());
    ui->buttonVisualizerBackgroundColor->setStyleSheet(qss);
}

void settingsWindow::on_sliderScrollerAmplitude_valueChanged(const int value) const {
    mainWindow->getEffect()->setAmplitude(value);
    ui->labelScrollerAmplitudeValue->setText(QString::number(value));
}

void settingsWindow::on_sliderScrollerFrequency_valueChanged(const int value) const {
    mainWindow->getEffect()->setSinusFrequency(value / 10000.0f);
    ui->labelScrollerFrequencyValue->setText(QString::number(value / 10000.0f));
}

void settingsWindow::on_sliderScrollerSinusSpeed_valueChanged(const int value) const {
    mainWindow->getEffect()->setSinusSpeed(value / 100.0f);
    ui->labelScrollerSinusSpeedValue->setText(QString::number(value / 100.0f));
}

void settingsWindow::on_sliderScrollerScrollSpeed_valueChanged(const int value) const {
    mainWindow->getEffect()->setScrollSpeed(value);
    ui->labelScrollerScrollSpeedValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRotatingObjectFocalLength_valueChanged(const int value) const {
    mainWindow->getEffect()->setRotatingObjectFocalLength(value);
    ui->labelRotatingObjectFocalLengthValue->setText(QString::number(static_cast<int>(value / 500.0 * 100)));
}

void settingsWindow::on_sliderRotatingObjectOrbitSize_valueChanged(const int value) const {
    mainWindow->getEffect()->setRotatingObjectOrbitSize(value);
    ui->labelRotatingObjectOrbitSizeValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRotatingObjectOrbitSpeed_valueChanged(const int value) const {
    mainWindow->getEffect()->setRotatingObjectOrbitSpeed(value);
    ui->labelRotatingObjectOrbitSpeedValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRotatingObjectModelSize_valueChanged(const int value) const {
    mainWindow->getEffect()->setRotatingObjectSize(value);
    ui->labelRotatingObjectModelSizeValue->setText(QString::number(value));
}

void settingsWindow::on_sliderScrollerFontXScale_valueChanged(const int value) const {
    mainWindow->getEffect()->setFontScaleX(value);
    ui->labelScrollerFontXScaleValue->setText(QString::number(value));
}

void settingsWindow::on_sliderScrollerFontYScale_valueChanged(const int value) const {
    mainWindow->getEffect()->setFontScaleY(value);
    ui->labelScrollerFontYScaleValue->setText(QString::number(value));
}

void settingsWindow::on_buttonVuMeterPeakColor_clicked() {
    const QColor oldColor(mainWindow->getColorVisualizerPeakColor());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorVisualizerPeakColor(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonVuMeterPeakColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_checkBoxVuMeterPeaks_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setVuMeterPeaksEnabled(isChecked);
    ui->checkBoxVuMeterPeaks->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);

    mainWindow->setVuMeterPeaksEnabled(isChecked);
    ui->labelVuMeterPeakColor->setEnabled(isChecked);
    ui->buttonVuMeterPeakColor->setEnabled(isChecked);
    ui->labelVuMeterPeakHeight->setEnabled(isChecked);
    ui->sliderVuMeterPeakHeight->setEnabled(isChecked);
    ui->labelVuMeterPeakHeightValue->setEnabled(isChecked);
}

void settingsWindow::on_sliderVuMeterPeakHeight_valueChanged(const int value) const {
    mainWindow->setVuMeterPeaksHeight(value);
    ui->labelVuMeterPeakHeightValue->setText(QString::number(value));
}

void settingsWindow::on_sliderVuMeterWidth_valueChanged(const int value) const {
    mainWindow->getEffect()->setVuMeterWidth(value);
    ui->labelVuMeterWidthValue->setText(QString::number(value) + "%");
}

void settingsWindow::on_sliderScrollerVerticalPosition_valueChanged(const int value) const {
    mainWindow->getEffect()->setVerticalScrollPosition(value);
    ui->labelScrollerVerticalPositionValue->setText(QString::number(value));
}

void settingsWindow::on_checkBoxReflectionEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setReflectionEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelReflectionColor->setEnabled(true);
        ui->buttonReflectionColor->setEnabled(true);
        ui->labelReflectionOpacity->setEnabled(true);
        ui->sliderReflectionOpacity->setEnabled(true);
        ui->labelReflectionOpacityValue->setEnabled(true);
    } else {
        ui->checkBoxReflectionEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelReflectionColor->setEnabled(false);
        ui->buttonReflectionColor->setEnabled(false);
        ui->labelReflectionOpacity->setEnabled(false);
        ui->sliderReflectionOpacity->setEnabled(false);
        ui->labelReflectionOpacityValue->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxStarsEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setStarsEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelStarfieldAmount->setEnabled(true);
        ui->sliderStarfieldAmount->setEnabled(true);
        ui->labelStarfieldAmountValue->setEnabled(true);
        ui->sliderStarfieldSpeed->setEnabled(true);
        ui->labelStarfieldSpeed->setEnabled(true);
        ui->labelStarfieldSpeedValue->setEnabled(true);
        ui->comboBoxStarfieldDirection->setEnabled(true);
        ui->labelStarfieldDirection->setEnabled(true);
    } else {
        ui->checkBoxStarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelStarfieldAmount->setEnabled(false);
        ui->sliderStarfieldAmount->setEnabled(false);
        ui->labelStarfieldAmountValue->setEnabled(false);
        ui->sliderStarfieldSpeed->setEnabled(false);
        ui->labelStarfieldSpeed->setEnabled(false);
        ui->labelStarfieldSpeedValue->setEnabled(false);
        ui->comboBoxStarfieldDirection->setEnabled(false);
        ui->labelStarfieldDirection->setEnabled(false);
    }
}

void settingsWindow::on_buttonReflectionColor_clicked() {
    const QColor oldColor(mainWindow->getEffect()->getReflectionColor());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->getEffect()->setScrollerReflectionColor(newColor.name());

        const QString qss = QString("background-color: %1").arg(mainWindow->getEffect()->getReflectionColor());
        ui->buttonReflectionColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonRotatingObjectMaterialColor_clicked() {
    const QColor oldColor(mainWindow->getEffect()->getRotatingObjectColor());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->getEffect()->setRotatingObjectColor(newColor.name());

        const QString qss = QString("background-color: %1").arg(mainWindow->getEffect()->getRotatingObjectColor());
        ui->buttonRotatingObjectMaterialColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_buttonRotatingObjectWireframeColor_clicked() {
    const QColor oldColor(mainWindow->getEffect()->getRotatingObjectColorWireframe());

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->getEffect()->setRotatingObjectColorWireframe(newColor.name());

        const QString qss = QString("background-color: %1").arg(
            mainWindow->getEffect()->getRotatingObjectColorWireframe());
        ui->buttonRotatingObjectWireframeColor->setStyleSheet(qss);
    }
}

void settingsWindow::loadBitmapFont(const QString &file) const {
    ui->albumGridScrollerFont->setVisible(false);
    mainWindow->getEffect()->setScrollerFont(file);

    const int extensionPos = file.lastIndexOf('.');
    const QString thumb(file.left(extensionPos) + ".thumb.png");
    ui->buttonScrollerFontImage->setIcon(QIcon(thumb));
}

void settingsWindow::loadBitmapFontPrinter(const QString &file) const {
    ui->albumGridPrinterFont->setVisible(false);
    mainWindow->getEffect()->setPrinterFont(file);

    const int extensionPos = file.lastIndexOf('.');
    const QString thumb(file.left(extensionPos) + ".thumb.png");
    ui->buttonPrinterFontImage->setIcon(QIcon(thumb));
}

void settingsWindow::on_sliderReflectionOpacity_valueChanged(const int value) const {
    mainWindow->getEffect()->setReflectionOpacity(value);
    ui->labelReflectionOpacityValue->setText(QString::number(value) + "%");
}

void settingsWindow::on_sliderVuMeterOpacity_valueChanged(const int value) const {
    mainWindow->getEffect()->setVuMeterOpacity(value);
    ui->labelVuMeterOpacityValue->setText(QString::number(value) + "%");
}

void settingsWindow::on_sliderPrinterFontXScale_valueChanged(const int value) const {
    mainWindow->getEffect()->setPrinterFontScaleX(value);
    ui->labelPrinterFontXScaleValue->setText(QString::number(value));
}

void settingsWindow::on_sliderPrinterFontYScale_valueChanged(const int value) const {
    mainWindow->getEffect()->setPrinterFontScaleY(value);
    ui->labelPrinterFontYScaleValue->setText(QString::number(value));
}

void settingsWindow::on_checkBoxOnlyOneInstance_toggled(const bool isChecked) const {
    mainWindow->setAllowOnlyOneInstanceEnabled(isChecked);
    ui->checkBoxEnqueueItems->setEnabled(isChecked);

    if (ui->checkBoxOnlyOneInstance->isChecked()) {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-on"]);
        ui->checkBoxEnqueueItems->setIcon(
            mainWindow->icons[ui->checkBoxEnqueueItems->isChecked() ? "checkbox-on" : "checkbox-off"]);
    } else {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-off"]);
        ui->checkBoxEnqueueItems->setIcon(
            mainWindow->icons[ui->checkBoxEnqueueItems->isChecked()
                                  ? "checkbox-on-disabled"
                                  : "checkbox-off-disabled"]);
    }
}

void settingsWindow::on_sliderUadePanning_valueChanged(const int value) const {
    ui->labelUadePanningValue->setText(QString::number(static_cast<float>(value) / 10, 'f', 1));
}

void settingsWindow::on_sliderUadeSilenceTimeOut_valueChanged(const int value) const {
    ui->labelUadeSilenceTimeOutValue->setText(QString::number(value) + "s");
}

void settingsWindow::on_checkBoxUadeSilenceTimeout_toggled(const bool isChecked) const {
    if (isChecked) {
        ui->checkBoxUadeSilenceTimeout->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelUadeSilenceTimeOutValue->setEnabled(true);
        ui->sliderUadeSilenceTimeOut->setEnabled(true);
    } else {
        ui->checkBoxUadeSilenceTimeout->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelUadeSilenceTimeOutValue->setEnabled(false);
        ui->sliderUadeSilenceTimeOut->setEnabled(false);
    }
}

void settingsWindow::on_buttonUadeSonglengthsBrowse_clicked() {
    QString startFolder = ui->lineEditUadeSonglengthsPath->text();

    if (startFolder.compare("/uade.md5") == 0) {
        startFolder = dataPath + PLUGIN_uade_DIR + "/uade.md5";
    }

    if (const QString file = QFileDialog::getOpenFileName(this, "Choose your uade.md5", startFolder, "*.md5");
        !file.isEmpty()) {
        if (file.compare(dataPath + PLUGIN_uade_DIR + "/uade.md5") == 0) {
            ui->lineEditUadeSonglengthsPath->setText("/uade.md5");
        } else {
            ui->lineEditUadeSonglengthsPath->setText(file);
        }
    }
}

void settingsWindow::on_buttonAppearanceSelectionColor_clicked() {
    const QColor oldColor(mainWindow->getColorSelection().left(7));

    if (const QColor newColor = QColorDialog::getColor(oldColor, this, "Select color");
        newColor.isValid()) {
        mainWindow->setColorSelection(newColor.name());

        const QString qss = QString("background-color: %1").arg(newColor.name());
        ui->buttonAppearanceSelectionColor->setStyleSheet(qss);
    }
}

void settingsWindow::on_checkBoxMilliseconds_toggled(const bool isChecked) const {
    mainWindow->setDisplayMilliseconds(isChecked);
    ui->checkBoxMilliseconds->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_sliderLibopenmptStereoSeparation_valueChanged(const int value) const {
    const int valueAdjusted = value / 2;

    ui->sliderLibopenmptStereoSeparation->blockSignals(true);
    ui->sliderLibopenmptStereoSeparation->setValue(valueAdjusted * 2);
    ui->labelLibopenmptStereoSeparationValue->setText(QString::number(valueAdjusted) + "%");
    ui->sliderLibopenmptStereoSeparation->blockSignals(false);
}

void settingsWindow::on_checkBoxLibopenmptContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxLibopenmptContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxEnqueueItems_toggled(const bool isChecked) const {
    mainWindow->setEnqueueItems(isChecked);
    ui->checkBoxEnqueueItems->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxShowLoopPoints_toggled(const bool isChecked) const {
    ui->checkBoxShowLoopPoints->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
    mainWindow->showCheckBoxLoopPoints(isChecked);
}

void settingsWindow::on_checkBoxVuMeterEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setVuMeterEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxVuMeterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelVuMeterBottomColor->setEnabled(true);
        ui->labelVuMeterTopColor->setEnabled(true);
        ui->labelVuMeterMiddleColor->setEnabled(true);
        ui->buttonVuMeterTopColor->setEnabled(true);
        ui->buttonVuMeterMiddleColor->setEnabled(true);
        ui->buttonVuMeterBottomColor->setEnabled(true);
        ui->labelVuMeterWidth->setEnabled(true);
        ui->labelVuMeterOpacity->setEnabled(true);
        ui->sliderVuMeterWidth->setEnabled(true);
        ui->sliderVuMeterOpacity->setEnabled(true);
        ui->labelVuMeterWidthValue->setEnabled(true);
        ui->labelVuMeterOpacityValue->setEnabled(true);
        ui->checkBoxVuMeterPeaks->setEnabled(true);
        ui->labelVuMeterPeakColor->setEnabled(true);
        ui->labelVuMeterPeakHeight->setEnabled(true);
        ui->checkBoxVuMeterPeaks->setEnabled(true);
        ui->buttonVuMeterPeakColor->setEnabled(true);
        ui->sliderVuMeterPeakHeight->setEnabled(true);
        ui->labelVuMeterPeakHeightValue->setEnabled(true);
    } else {
        ui->checkBoxVuMeterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelVuMeterBottomColor->setEnabled(false);
        ui->labelVuMeterTopColor->setEnabled(false);
        ui->labelVuMeterMiddleColor->setEnabled(false);
        ui->buttonVuMeterTopColor->setEnabled(false);
        ui->buttonVuMeterMiddleColor->setEnabled(false);
        ui->buttonVuMeterBottomColor->setEnabled(false);
        ui->labelVuMeterWidth->setEnabled(false);
        ui->labelVuMeterOpacity->setEnabled(false);
        ui->sliderVuMeterWidth->setEnabled(false);
        ui->sliderVuMeterOpacity->setEnabled(false);
        ui->labelVuMeterWidthValue->setEnabled(false);
        ui->labelVuMeterOpacityValue->setEnabled(false);
        ui->checkBoxVuMeterPeaks->setEnabled(false);
        ui->labelVuMeterPeakColor->setEnabled(false);
        ui->labelVuMeterPeakHeight->setEnabled(false);
        ui->checkBoxVuMeterPeaks->setEnabled(false);
        ui->buttonVuMeterPeakColor->setEnabled(false);
        ui->sliderVuMeterPeakHeight->setEnabled(false);
        ui->labelVuMeterPeakHeightValue->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxScrollerEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setScrollerEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelScrollerAmplitude->setEnabled(true);
        ui->sliderScrollerAmplitude->setEnabled(true);
        ui->labelScrollerAmplitudeValue->setEnabled(true);
        ui->labelScrollerFrequency->setEnabled(true);
        ui->sliderScrollerFrequency->setEnabled(true);
        ui->labelScrollerFrequencyValue->setEnabled(true);
        ui->labelScrollerSinusSpeed->setEnabled(true);
        ui->sliderScrollerSinusSpeed->setEnabled(true);
        ui->labelScrollerSinusSpeedValue->setEnabled(true);
        ui->labelScrollerScrollSpeed->setEnabled(true);
        ui->sliderScrollerScrollSpeed->setEnabled(true);
        ui->labelScrollerScrollSpeedValue->setEnabled(true);
        ui->labelScrollerVerticalPosition->setEnabled(true);
        ui->sliderScrollerVerticalPosition->setEnabled(true);
        ui->labelScrollerVerticalPositionValue->setEnabled(true);
        ui->labelScrollerFontXScale->setEnabled(true);
        ui->sliderScrollerFontXScale->setEnabled(true);
        ui->labelScrollerFontXScaleValue->setEnabled(true);
        ui->labelScrollerFontYScale->setEnabled(true);
        ui->sliderScrollerFontYScale->setEnabled(true);
        ui->labelScrollerFontYScaleValue->setEnabled(true);
        ui->checkBoxScrollerSinusFontScaling->setEnabled(true);
        ui->checkBoxScrollerCustomTextEnabled->setEnabled(true);

        if (ui->checkBoxScrollerCustomTextEnabled->isChecked())
            ui->textEditScrollerCustomText->setEnabled(true);
        else {
            ui->textEditScrollerCustomText->setEnabled(false);
        }
    } else {
        ui->checkBoxScrollerEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelScrollerAmplitude->setEnabled(false);
        ui->sliderScrollerAmplitude->setEnabled(false);
        ui->labelScrollerAmplitudeValue->setEnabled(false);
        ui->labelScrollerFrequency->setEnabled(false);
        ui->sliderScrollerFrequency->setEnabled(false);
        ui->labelScrollerFrequencyValue->setEnabled(false);
        ui->labelScrollerSinusSpeed->setEnabled(false);
        ui->sliderScrollerSinusSpeed->setEnabled(false);
        ui->labelScrollerSinusSpeedValue->setEnabled(false);
        ui->labelScrollerScrollSpeed->setEnabled(false);
        ui->sliderScrollerScrollSpeed->setEnabled(false);
        ui->labelScrollerScrollSpeedValue->setEnabled(false);
        ui->labelScrollerVerticalPosition->setEnabled(false);
        ui->sliderScrollerVerticalPosition->setEnabled(false);
        ui->labelScrollerVerticalPositionValue->setEnabled(false);
        ui->labelScrollerFontXScale->setEnabled(false);
        ui->sliderScrollerFontXScale->setEnabled(false);
        ui->labelScrollerFontXScaleValue->setEnabled(false);
        ui->labelScrollerFontYScale->setEnabled(false);
        ui->sliderScrollerFontYScale->setEnabled(false);
        ui->labelScrollerFontYScaleValue->setEnabled(false);
        ui->checkBoxScrollerSinusFontScaling->setEnabled(false);
        ui->checkBoxScrollerCustomTextEnabled->setEnabled(false);
        ui->textEditScrollerCustomText->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxPrinterEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setPrinterEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->sliderPrinterFontXScale->setEnabled(true);
        ui->sliderPrinterFontYScale->setEnabled(true);
        ui->labelPrinterFontXScale->setEnabled(true);
        ui->labelPrinterFontYScale->setEnabled(true);
        ui->labelPrinterFontXScaleValue->setEnabled(true);
        ui->labelPrinterFontYScaleValue->setEnabled(true);
    } else {
        ui->checkBoxPrinterEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->sliderPrinterFontXScale->setEnabled(false);
        ui->sliderPrinterFontYScale->setEnabled(false);
        ui->labelPrinterFontXScale->setEnabled(false);
        ui->labelPrinterFontYScale->setEnabled(false);
        ui->labelPrinterFontXScaleValue->setEnabled(false);
        ui->labelPrinterFontYScaleValue->setEnabled(false);
    }
}

void settingsWindow::on_sliderStarfieldAmount_valueChanged(const int value) const {
    mainWindow->getEffect()->setNumberOfStars(value);
    ui->labelStarfieldAmountValue->setText(QString::number(value));
}

void settingsWindow::on_sliderStarfieldSpeed_valueChanged(const int value) const {
    mainWindow->getEffect()->setStarSpeed(value);
    ui->labelStarfieldSpeedValue->setText(QString::number(value));
}

void settingsWindow::on_comboBoxStarfieldDirection_textActivated(const QString &arg1) const {
    const QString selected = ui->comboBoxStarfieldDirection->itemData(ui->comboBoxStarfieldDirection->currentIndex()).
            toString();
    mainWindow->getEffect()->setStarsDirection(selected);
}

void settingsWindow::on_comboBoxRotatingObjectModel_textActivated(const QString &arg1) const {
    const QString selected = ui->comboBoxRotatingObjectModel->itemData(ui->comboBoxRotatingObjectModel->currentIndex()).
            toString();
    mainWindow->getEffect()->setRotatingObjectModel(selected);
}

void settingsWindow::on_comboBoxRotatingObjectMaterial_textActivated(const QString &arg1) const {
    const QString selected = ui->comboBoxRotatingObjectMaterial->itemData(
        ui->comboBoxRotatingObjectMaterial->currentIndex()).toString();
    mainWindow->getEffect()->setRotatingObjectMaterial(selected);
}

void settingsWindow::on_checkBoxScrollerSinusFontScaling_toggled(const bool isChecked) const {
    ui->checkBoxScrollerSinusFontScaling->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
    mainWindow->getEffect()->setSinusFontScalingEnabled(isChecked);
}

void settingsWindow::on_buttonScrollerFontImage_clicked() const {
    if (ui->albumGridScrollerFont->isHidden()) {
        ui->albumGridScrollerFont->setVisible(true);
    } else {
        ui->albumGridScrollerFont->setVisible(false);
    }
}

void settingsWindow::on_buttonPrinterFontImage_clicked() const {
    if (ui->albumGridPrinterFont->isHidden()) {
        ui->albumGridPrinterFont->setVisible(true);
    } else {
        ui->albumGridPrinterFont->setVisible(false);
    }
}

void settingsWindow::on_sliderVisualizerResolutionWidth_valueChanged(const int value) const {
    mainWindow->getEffect()->setResolutionWidth(value);
    ui->labelVisualizerResolutionWidthValue->setText(QString::number(value));
}

void settingsWindow::on_sliderVisualizerResolutionHeight_valueChanged(const int value) const {
    mainWindow->getEffect()->setResolutionHeight(value);
    ui->labelVisualizerResolutionHeightValue->setText(QString::number(value));
}

void settingsWindow::on_checkBoxVisualizerMaintainAspectRatio_toggled(const bool isChecked) const {
    ui->checkBoxVisualizerMaintainAspectRatio->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
    mainWindow->getEffect()->setKeepAspectRatio(isChecked);
}

void settingsWindow::on_comboBoxLibsidplayfpHvscSonglengthsUpdate_textActivated(const QString &arg1) const {
    const QString selected = ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->itemData(
        ui->comboBoxLibsidplayfpHvscSonglengthsUpdate->currentIndex()).toString();
    mainWindow->setBundledHvscSonglengthsUpdateFrequency(selected);
}

void settingsWindow::on_buttonLibsidplayfpHvscSonglengthsDownload_clicked() {
    const QUrl imageUrl(PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_URL);
    mainWindow->filedownloader = new FileDownloader(imageUrl, this);
    ui->buttonLibsidplayfpHvscSonglengthsDownload->setEnabled(true);
    ui->buttonLibsidplayfpHvscSonglengthsDownload->setText("Downloading...");

    connect(mainWindow->filedownloader, SIGNAL(downloaded()), this, SLOT(downloadHvscSonglengthsComplete()));
}

void settingsWindow::downloadHvscSonglengthsComplete() const {
    if (mainWindow->filedownloader->downloadedData().isEmpty()) {
        mainWindow->addDebugText("Failed to download " + mainWindow->filedownloader->getUrl().toString());
        return;
    }

    const QString hvscSonglengthsDownloadPath = userPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH;

    QFile file(hvscSonglengthsDownloadPath);

    if (!file.open(QIODevice::ReadWrite)) {
        mainWindow->addDebugText("Couldn't write to file " + file.fileName());
        return;
    }

    QTextStream stream(&file);
    stream << mainWindow->filedownloader->downloadedData();
    file.close();

    mainWindow->bundledHvscSonglengthsDownloadEpoch = QDateTime::currentSecsSinceEpoch();
    mainWindow->setBundledHvscSonglengthsPath(hvscSonglengthsDownloadPath);

    if (ui->lineEditLibsidplayfpHvscSonglengthsPath->text()
        .compare(dataPath + PLUGIN_libsidplayfp_HVSC_SONGLENGTHS_PATH) == 0) {
        setUiLineEditLibsidplayfpHvscSonglengthsPath(hvscSonglengthsDownloadPath);
        mainWindow->setHvscSonglengthsPath(hvscSonglengthsDownloadPath);
    }

    const QDateTime qdt = QDateTime::fromSecsSinceEpoch(mainWindow->getBundledHvscSonglengthsDownloadEpoch());
    ui->labelLibsidplayfpHvscSonglengthsDownloadDetails->setText(
        "Downloaded to " + hvscSonglengthsDownloadPath + " at " + qdt.toString("yyyy-MM-dd hh:mm:ss"));
    ui->buttonLibsidplayfpHvscSonglengthsDownload->setEnabled(true);
    ui->buttonLibsidplayfpHvscSonglengthsDownload->setText("Download now");

    QSettings settings(userPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("Plugins/libsidplayfpBundledHvscSonglengthsPath", hvscSonglengthsDownloadPath);
    settings.setValue("Plugins/libsidplayfpBundledHvscSonglengthsDownloadEpoch",
                      mainWindow->bundledHvscSonglengthsDownloadEpoch);
}

void settingsWindow::on_sliderRasterBarsAmount_valueChanged(const int value) const {
    mainWindow->getEffect()->setNumberOfRasterBars(value);
    ui->labelRasterBarsAmountValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRasterBarsVerticalSpacing_valueChanged(const int value) const {
    mainWindow->getEffect()->setRasterBarsVerticalSpacing(value);
    ui->labelRasterBarsVerticalSpacingValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRasterBarsHeight_valueChanged(const int value) const {
    mainWindow->getEffect()->setRasterBarsBarHeight(value);
    ui->labelRasterBarsHeightValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRasterBarsSpeed_valueChanged(const int value) const {
    mainWindow->getEffect()->setRasterBarsSpeed(value);
    ui->labelRasterBarsSpeedValue->setText(QString::number(value));
}

void settingsWindow::on_sliderRasterBarsOpacity_valueChanged(const int value) const {
    mainWindow->getEffect()->setRasterBarsOpacity(value);
    ui->labelRasterBarsOpacityValue->setText(QString::number(value));
}

void settingsWindow::on_checkBoxRotatingObjectEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setRotatingObjectEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxRotatingObjectEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->sliderRotatingObjectOrbitSize->setEnabled(true);
        ui->labelRotatingObjectOrbitSize->setEnabled(true);
        ui->labelRotatingObjectOrbitSizeValue->setEnabled(true);
        ui->sliderRotatingObjectOrbitSpeed->setEnabled(true);
        ui->labelRotatingObjectOrbitSpeed->setEnabled(true);
        ui->labelRotatingObjectOrbitSpeedValue->setEnabled(true);
        ui->sliderRotatingObjectFocalLength->setEnabled(true);
        ui->labelRotatingObjectFocalLengthValue->setEnabled(true);
        ui->labelRotatingObjectFocalLength->setEnabled(true);
        ui->labelRotatingObjectModelSize->setEnabled(true);
        ui->buttonRotatingObjectWireframeColor->setEnabled(true);
        ui->labelRotatingObjectWireframeColor->setEnabled(true);
        ui->labelRotatingObjectMaterialColor->setEnabled(true);
        ui->checkBoxRotatingObjectOrbit->setEnabled(true);
        ui->checkBoxRotatingObjectWireframeEnabled->setEnabled(true);
        ui->comboBoxRotatingObjectMaterial->setEnabled(true);
        ui->sliderRotatingObjectModelSize->setEnabled(true);
        ui->labelRotatingObjectMaterial->setEnabled(true);
        ui->labelRotatingObjectModel->setEnabled(true);
        ui->comboBoxRotatingObjectModel->setEnabled(true);
        ui->buttonRotatingObjectMaterialColor->setEnabled(true);
        ui->labelRotatingObjectModelSizeValue->setEnabled(true);
    } else {
        ui->checkBoxRotatingObjectEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->sliderRotatingObjectOrbitSize->setEnabled(false);
        ui->labelRotatingObjectOrbitSize->setEnabled(false);
        ui->labelRotatingObjectOrbitSizeValue->setEnabled(false);
        ui->sliderRotatingObjectOrbitSpeed->setEnabled(false);
        ui->labelRotatingObjectOrbitSpeed->setEnabled(false);
        ui->labelRotatingObjectOrbitSpeedValue->setEnabled(false);
        ui->sliderRotatingObjectFocalLength->setEnabled(false);
        ui->labelRotatingObjectFocalLengthValue->setEnabled(false);
        ui->labelRotatingObjectFocalLength->setEnabled(false);
        ui->labelRotatingObjectModelSize->setEnabled(false);
        ui->buttonRotatingObjectWireframeColor->setEnabled(false);
        ui->labelRotatingObjectWireframeColor->setEnabled(false);
        ui->labelRotatingObjectMaterialColor->setEnabled(false);
        ui->checkBoxRotatingObjectOrbit->setEnabled(false);
        ui->checkBoxRotatingObjectWireframeEnabled->setEnabled(false);
        ui->comboBoxRotatingObjectMaterial->setEnabled(false);
        ui->sliderRotatingObjectModelSize->setEnabled(false);
        ui->labelRotatingObjectMaterial->setEnabled(false);
        ui->labelRotatingObjectModel->setEnabled(false);
        ui->comboBoxRotatingObjectModel->setEnabled(false);
        ui->buttonRotatingObjectMaterialColor->setEnabled(false);
        ui->labelRotatingObjectModelSizeValue->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxRotatingObjectOrbit_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setRotatingObjectOrbit(isChecked);

    if (isChecked) {
        ui->checkBoxRotatingObjectOrbit->setIcon(mainWindow->icons["checkbox-on"]);
        ui->sliderRotatingObjectOrbitSize->setEnabled(true);
        ui->labelRotatingObjectOrbitSize->setEnabled(true);
        ui->labelRotatingObjectOrbitSizeValue->setEnabled(true);
        ui->sliderRotatingObjectOrbitSpeed->setEnabled(true);
        ui->labelRotatingObjectOrbitSpeed->setEnabled(true);
        ui->labelRotatingObjectOrbitSpeedValue->setEnabled(true);
    } else {
        ui->checkBoxRotatingObjectOrbit->setIcon(mainWindow->icons["checkbox-off"]);
        ui->sliderRotatingObjectOrbitSize->setEnabled(false);
        ui->labelRotatingObjectOrbitSize->setEnabled(false);
        ui->labelRotatingObjectOrbitSizeValue->setEnabled(false);
        ui->sliderRotatingObjectOrbitSpeed->setEnabled(false);
        ui->labelRotatingObjectOrbitSpeed->setEnabled(false);
        ui->labelRotatingObjectOrbitSpeedValue->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxRotatingObjectWireframeEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setRotatingObjectWireframeEnabled(isChecked);
    ui->checkBoxRotatingObjectWireframeEnabled->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxRasterBarsEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setRasterBarsEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxRasterBarsEnabled->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelRasterBarsAmount->setEnabled(true);
        ui->sliderRasterBarsAmount->setEnabled(true);
        ui->labelRasterBarsAmountValue->setEnabled(true);
        ui->labelRasterBarsHeight->setEnabled(true);
        ui->sliderRasterBarsHeight->setEnabled(true);
        ui->labelRasterBarsHeightValue->setEnabled(true);
        ui->labelRasterBarsVerticalSpacing->setEnabled(true);
        ui->sliderRasterBarsVerticalSpacing->setEnabled(true);
        ui->labelRasterBarsVerticalSpacingValue->setEnabled(true);
        ui->labelRasterBarsSpeed->setEnabled(true);
        ui->sliderRasterBarsSpeed->setEnabled(true);
        ui->labelRasterBarsSpeedValue->setEnabled(true);
        ui->labelRasterBarsOpacity->setEnabled(true);
        ui->sliderRasterBarsOpacity->setEnabled(true);
        ui->labelRasterBarsOpacityValue->setEnabled(true);
    } else {
        ui->checkBoxRasterBarsEnabled->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelRasterBarsAmount->setEnabled(false);
        ui->sliderRasterBarsAmount->setEnabled(false);
        ui->labelRasterBarsAmountValue->setEnabled(false);
        ui->labelRasterBarsHeight->setEnabled(false);
        ui->sliderRasterBarsHeight->setEnabled(false);
        ui->labelRasterBarsHeightValue->setEnabled(false);
        ui->labelRasterBarsVerticalSpacing->setEnabled(false);
        ui->sliderRasterBarsVerticalSpacing->setEnabled(false);
        ui->labelRasterBarsVerticalSpacingValue->setEnabled(false);
        ui->labelRasterBarsSpeed->setEnabled(false);
        ui->sliderRasterBarsSpeed->setEnabled(false);
        ui->labelRasterBarsSpeedValue->setEnabled(false);
        ui->labelRasterBarsOpacity->setEnabled(false);
        ui->sliderRasterBarsOpacity->setEnabled(false);
        ui->labelRasterBarsOpacityValue->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxScrollerCustomTextEnabled_toggled(const bool isChecked) const {
    mainWindow->getEffect()->setCustomScrolltextEnabled(isChecked);
    ui->checkBoxScrollerCustomTextEnabled->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);

    ui->textEditScrollerCustomText->setEnabled(isChecked);

    updateScrollText();
}

void settingsWindow::updateScrollText() const {
    mainWindow->getEffect()->setCustomScrolltext(ui->textEditScrollerCustomText->toPlainText());
    mainWindow->updateScrollText();
}

void settingsWindow::updateCheckBoxes() const {
    if (ui->checkBoxOnlyOneInstance->isChecked()) {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-on"]);
        ui->checkBoxEnqueueItems->setIcon(
            mainWindow->icons[ui->checkBoxEnqueueItems->isChecked() ? "checkbox-on" : "checkbox-off"]);
    } else {
        ui->checkBoxOnlyOneInstance->setIcon(mainWindow->icons["checkbox-off"]);
        ui->checkBoxEnqueueItems->setIcon(mainWindow->icons[ui->checkBoxEnqueueItems->isChecked()
                                                                ? "checkbox-on-disabled"
                                                                : "checkbox-off-disabled"]);
    }

    ui->checkBoxScrollerCustomTextEnabled->setIcon(
        mainWindow->icons[ui->checkBoxScrollerCustomTextEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxScrollerSinusFontScaling->setIcon(
        mainWindow->icons[ui->checkBoxScrollerSinusFontScaling->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxVuMeterPeaks->setIcon(
        mainWindow->icons[ui->checkBoxVuMeterPeaks->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxReflectionEnabled->setIcon(
        mainWindow->icons[ui->checkBoxReflectionEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxScrollerEnabled->setIcon(
        mainWindow->icons[ui->checkBoxScrollerEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxRasterBarsEnabled->setIcon(
        mainWindow->icons[ui->checkBoxRasterBarsEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxRotatingObjectEnabled->setIcon(
        mainWindow->icons[ui->checkBoxRotatingObjectEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxRotatingObjectWireframeEnabled->setIcon(
        mainWindow->icons[ui->checkBoxRotatingObjectWireframeEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxRotatingObjectOrbit->setIcon(
        mainWindow->icons[ui->checkBoxRotatingObjectOrbit->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxPrinterEnabled->setIcon(
        mainWindow->icons[ui->checkBoxPrinterEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxStarsEnabled->setIcon(
        mainWindow->icons[ui->checkBoxStarsEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxVisualizerMaintainAspectRatio->setIcon(
        mainWindow->icons[ui->checkBoxVisualizerMaintainAspectRatio->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxVuMeterEnabled->setIcon(
        mainWindow->icons[ui->checkBoxVuMeterEnabled->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxAdPlugContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxAdPlugContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxFurnaceContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxFurnaceContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxHighlyExperimentalContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxHighlyExperimentalContinuousPlayback->isChecked()
                              ? "checkbox-on"
                              : "checkbox-off"]);
    ui->checkBoxHighlyQuixoticContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxHighlyQuixoticContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxHivelyTrackerContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxHivelyTrackerContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxLibvgmContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxLibvgmContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxSndhPlayerContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxSndhPlayerContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxVgmstreamContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxVgmstreamContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxLibopenmptContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxLibopenmptContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxFmodSeamlessLoop->setIcon(
        mainWindow->icons[ui->checkBoxFmodSeamlessLoop->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxLibopenmptAmigaResampler->setIcon(
        mainWindow->icons[ui->checkBoxLibopenmptAmigaResampler->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxUadeFilterEmu->setIcon(
        mainWindow->icons[ui->checkBoxUadeFilterEmu->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxUadeSilenceTimeout->setIcon(
        mainWindow->icons[ui->checkBoxUadeSilenceTimeout->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxUadeContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxUadeContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxUadeSongLengths->setIcon(
        mainWindow->icons[ui->checkBoxUadeSongLengths->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->setIcon(
        mainWindow->icons[ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->isChecked()
                              ? "checkbox-on"
                              : "checkbox-off"]);
    ui->checkBoxLibsidplayfpContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxLibsidplayfpContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxLibxmpContinuousPlayback->setIcon(
        mainWindow->icons[ui->checkBoxLibxmpContinuousPlayback->isChecked() ? "checkbox-on" : "checkbox-off"]);

    if (ui->checkBoxSystray->isChecked()) {
        ui->checkBoxSystray->setIcon(mainWindow->icons["checkbox-on"]);
        ui->checkBoxMinimizeToSystray->setIcon(
            mainWindow->icons[ui->checkBoxMinimizeToSystray->isChecked() ? "checkbox-on" : "checkbox-off"]);
    } else {
        ui->checkBoxSystray->setIcon(mainWindow->icons["checkbox-off"]);
        ui->checkBoxMinimizeToSystray->setIcon(
            mainWindow->icons[ui->checkBoxMinimizeToSystray->isChecked()
                                  ? "checkbox-on-disabled"
                                  : "checkbox-off-disabled"]);
    }

    ui->checkBoxMenuBarHidden->setIcon(
        mainWindow->icons[ui->checkBoxMenuBarHidden->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxNormalizer->setIcon(
        mainWindow->icons[ui->checkBoxNormalizer->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxReverb->setIcon(mainWindow->icons[ui->checkBoxReverb->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxDefaultAudioLevel->setIcon(
        mainWindow->icons[ui->checkBoxDefaultAudioLevel->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxMilliseconds->setIcon(
        mainWindow->icons[ui->checkBoxMilliseconds->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxShowLoopPoints->setIcon(
        mainWindow->icons[ui->checkBoxShowLoopPoints->isChecked() ? "checkbox-on" : "checkbox-off"]);
    ui->checkBoxDefaultTrackLength->setIcon(
        mainWindow->icons[ui->checkBoxDefaultTrackLength->isChecked() ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::forceUpdateToSliders() const {
    on_sliderVuMeterOpacity_valueChanged(ui->sliderVuMeterOpacity->value());
    on_sliderScrollerAmplitude_valueChanged(ui->sliderScrollerAmplitude->value());
    on_sliderScrollerFrequency_valueChanged(ui->sliderScrollerFrequency->value());
    on_sliderScrollerSinusSpeed_valueChanged(ui->sliderScrollerSinusSpeed->value());
    on_sliderScrollerScrollSpeed_valueChanged(ui->sliderScrollerScrollSpeed->value());
    on_sliderScrollerVerticalPosition_valueChanged(ui->sliderScrollerVerticalPosition->value());
    on_sliderScrollerFontXScale_valueChanged(ui->sliderScrollerFontXScale->value());
    on_sliderScrollerFontYScale_valueChanged(ui->sliderScrollerFontYScale->value());
    on_sliderPrinterFontXScale_valueChanged(ui->sliderPrinterFontXScale->value());
    on_sliderPrinterFontYScale_valueChanged(ui->sliderPrinterFontYScale->value());
    on_sliderVuMeterPeakHeight_valueChanged(ui->sliderVuMeterPeakHeight->value());
    on_sliderReflectionOpacity_valueChanged(ui->sliderReflectionOpacity->value());
    on_sliderAppearancePlaylistRowHeight_valueChanged(ui->sliderAppearancePlaylistRowHeight->value());
    on_sliderAppearancePlaylistItemRowHeight_valueChanged(ui->sliderAppearancePlaylistItemRowHeight->value());
    on_sliderAppearanceNowPlayingFontSize_valueChanged(ui->sliderAppearanceNowPlayingFontSize->value());
    on_sliderDefaultAudioLevel_valueChanged(ui->sliderDefaultAudioLevel->value());
    on_sliderNormalizerFadeTime_valueChanged(ui->sliderNormalizerFadeTime->value());
    on_sliderNormalizerMaxAmp_valueChanged(ui->sliderNormalizerMaxAmp->value());
    on_sliderNormalizerThreshold_valueChanged(ui->sliderNormalizerThreshold->value());
    on_sliderStarfieldAmount_valueChanged(ui->sliderStarfieldAmount->value());
    on_sliderStarfieldSpeed_valueChanged(ui->sliderStarfieldSpeed->value());
    on_sliderVisualizerResolutionWidth_valueChanged(ui->sliderVisualizerResolutionWidth->value());
    on_sliderVisualizerResolutionHeight_valueChanged(ui->sliderVisualizerResolutionHeight->value());
    on_sliderRasterBarsHeight_valueChanged(ui->sliderRasterBarsHeight->value());
    on_sliderRasterBarsSpeed_valueChanged(ui->sliderRasterBarsSpeed->value());
    on_sliderRasterBarsOpacity_valueChanged(ui->sliderRasterBarsOpacity->value());
    on_sliderRasterBarsVerticalSpacing_valueChanged(ui->sliderRasterBarsVerticalSpacing->value());
    on_sliderRasterBarsAmount_valueChanged(ui->sliderRasterBarsAmount->value());
    on_sliderRotatingObjectFocalLength_valueChanged(ui->sliderRotatingObjectFocalLength->value());
    on_sliderRotatingObjectModelSize_valueChanged(ui->sliderRotatingObjectModelSize->value());
}

void settingsWindow::on_checkBoxAdPlugContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxAdPlugContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxFurnaceContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxFurnaceContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxHighlyExperimentalContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxHighlyExperimentalContinuousPlayback->setIcon(
        mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxHighlyQuixoticContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxHighlyQuixoticContinuousPlayback->setIcon(
        mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxHivelyTrackerContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxHivelyTrackerContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxLibvgmContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxLibvgmContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxSndhPlayerContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxSndhPlayerContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxUadeContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxUadeContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxVgmstreamContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxVgmstreamContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxFmodSeamlessLoop_toggled(const bool isChecked) const {
    ui->checkBoxFmodSeamlessLoop->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxUadeSongLengths_toggled(const bool isChecked) const {
    if (isChecked) {
        ui->checkBoxUadeSongLengths->setIcon(mainWindow->icons["checkbox-on"]);
        ui->lineEditUadeSonglengthsPath->setEnabled(true);
        ui->buttonUadeSonglengthsBrowse->setEnabled(true);
    } else {
        ui->checkBoxUadeSongLengths->setIcon(mainWindow->icons["checkbox-off"]);
        ui->lineEditUadeSonglengthsPath->setEnabled(false);
        ui->buttonUadeSonglengthsBrowse->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxLibsidplayfpHvscSonglengthsEnabled_toggled(const bool isChecked) const {
    ui->lineEditLibsidplayfpHvscSonglengthsPath->setEnabled(isChecked);
    ui->buttonLibsidplayfpHvscSonglengthsBrowse->setEnabled(isChecked);
    ui->checkBoxLibsidplayfpHvscSonglengthsEnabled->
            setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxLibsidplayfpContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxLibsidplayfpContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxLibopenmptAmigaResampler_toggled(const bool isChecked) const {
    if (isChecked) {
        ui->checkBoxLibopenmptAmigaResampler->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelLibopenmptFilter->setEnabled(true);
        ui->comboBoxLibopenmptFilter->setEnabled(true);
    } else {
        ui->checkBoxLibopenmptAmigaResampler->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelLibopenmptFilter->setEnabled(false);
        ui->comboBoxLibopenmptFilter->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxLibxmpContinuousPlayback_toggled(const bool isChecked) const {
    ui->checkBoxLibxmpContinuousPlayback->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxOnlyOneInstance_clicked() {
    QMessageBox msgBox;
    msgBox.setText("You need to close all instances of BZR Player for this setting to have effect");
    msgBox.exec();
}

void settingsWindow::on_checkBoxUadeFilterEmu_toggled(const bool isChecked) const {
    if (isChecked) {
        ui->checkBoxUadeFilterEmu->setIcon(mainWindow->icons["checkbox-on"]);
        ui->labelUadeFilterEmuMode->setEnabled(true);
        ui->comboBoxUadeFilterEmuMode->setEnabled(true);
        ui->labelUadeLedFilter->setEnabled(true);
        ui->comboBoxUadeLedFilter->setEnabled(true);
    } else {
        ui->checkBoxUadeFilterEmu->setIcon(mainWindow->icons["checkbox-off"]);
        ui->labelUadeFilterEmuMode->setEnabled(false);
        ui->comboBoxUadeFilterEmuMode->setEnabled(false);
        ui->labelUadeLedFilter->setEnabled(false);
        ui->comboBoxUadeLedFilter->setEnabled(false);
    }
}

void settingsWindow::on_checkBoxSystray_toggled(const bool isChecked) const {
    ui->checkBoxMinimizeToSystray->setEnabled(isChecked);
    mainWindow->setSystrayOnMinimizeEnabled(isChecked);

    if (isChecked) {
        ui->checkBoxSystray->setIcon(mainWindow->icons["checkbox-on"]);
        ui->checkBoxMinimizeToSystray->setIcon(
            mainWindow->icons[ui->checkBoxMinimizeToSystray->isChecked() ? "checkbox-on" : "checkbox-off"]);
    } else {
        ui->checkBoxSystray->setIcon(mainWindow->icons["checkbox-off"]);
        ui->checkBoxMinimizeToSystray->setIcon(
            mainWindow->icons[ui->checkBoxMinimizeToSystray->isChecked()
                                  ? "checkbox-on-disabled"
                                  : "checkbox-off-disabled"]);
    }
}

void settingsWindow::on_checkBoxMinimizeToSystray_toggled(const bool isChecked) const {
    ui->checkBoxMinimizeToSystray->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_checkBoxMenuBarHidden_toggled(const bool isChecked) const {
    ui->checkBoxMenuBarHidden->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_sliderAppearanceNowPlayingFontSize_valueChanged(const int value) const {
    ui->labelAppearanceNowPlayingFontSizeValue->setText(QString::number(value) + "px");
    mainWindow->setNowPlayingFontSize(value);
}

void settingsWindow::on_checkBoxDefaultTrackLength_toggled(const bool isChecked) const {
    ui->sliderDefaultTrackLength->setEnabled(isChecked);
    ui->labelDefaultTrackLengthValue->setEnabled(isChecked);
    ui->checkBoxDefaultTrackLength->setIcon(mainWindow->icons[isChecked ? "checkbox-on" : "checkbox-off"]);
}

void settingsWindow::on_sliderDefaultTrackLength_valueChanged(const int value) const {
    ui->labelDefaultTrackLengthValue->setText(QString::number(value) + "m");
    mainWindow->setDefaultTrackLengthValue(value);
}
