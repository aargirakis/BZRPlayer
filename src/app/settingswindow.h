#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui
{
    class MainWindow;
    class settingsWindow;
}

class settingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingsWindow(QWidget* parent = nullptr);
    ~settingsWindow();

    bool eventFilter(QObject* obj, QEvent* event);
    void changeStyleSheetColor();
    void saveSidplaySettings();
    void setUiLineEditLibsidplayfpHvscSonglengthsPath(const QString& text);

private slots:
    void updateScrollText();

    void downloadHvscSonglengthsComplete();

    void on_sliderDefaultAudioLevel_sliderMoved(int position);

    void on_checkBoxDefaultAudioLevel_toggled(bool checked);

    void on_sliderDefaultAudioLevel_valueChanged(int value);

    void on_sliderNormalizerFadeTime_valueChanged(int value);

    void on_sliderNormalizerThreshold_valueChanged(int value);

    void on_sliderNormalizerMaxAmp_valueChanged(int value);

    void on_checkBoxNormalizer_toggled(bool checked);

    void on_checkBoxReverb_toggled(bool checked);


    void on_buttonOK_clicked();


    void on_comboBoxReverb_textActivated(const QString& arg1);


    void on_buttonLibsidplayfpHvscSonglengthsBrowse_clicked();
    void on_buttonGeneral_clicked();

    void on_buttonPlugins_clicked();

    void on_tableWidgetPlugins_itemClicked(QTableWidgetItem* item);

    void on_buttonAppearanceMainColor_clicked();

    void on_buttonAppearance_clicked();

    void on_buttonAppearanceMediumColor_clicked();

    void on_buttonAppearanceBackgroundColor_clicked();

    void on_buttonAppearanceBehindBackgroundColor_clicked();

    void on_buttonAppearanceMainTextColor_clicked();

    void on_buttonAppearanceButtonColor_clicked();

    void on_buttonAppearanceDimmedTextColor_clicked();

    void on_buttonAppearanceMainHoverColor_clicked();

    void on_buttonAppearanceButtonHoverColor_clicked();

    void on_buttonAppearanceResetColors_clicked();

    void on_buttonVisualizer_clicked();

    void on_buttonVuMeterTopColor_clicked();

    void on_buttonVuMeterBottomColor_clicked();

    void on_buttonVuMeterMiddleColor_clicked();

    void on_sliderAppearancePlaylistItemRowHeight_valueChanged(int value);

    void on_sliderAppearancePlaylistRowHeight_valueChanged(int value);

    void on_sliderScrollerAmplitude_valueChanged(int value);

    void on_sliderScrollerFrequency_valueChanged(int value);

    void on_sliderScrollerSinusSpeed_valueChanged(int value);

    void on_sliderScrollerScrollSpeed_valueChanged(int value);

    void on_sliderRotatingObjectFocalLength_valueChanged(int value);

    void on_sliderRotatingObjectOrbitSize_valueChanged(int value);

    void on_sliderRotatingObjectSpeed_valueChanged(int value);

    void on_sliderRotatingObjectModelSize_valueChanged(int value);

    void on_comboBoxRotatingObjectModel_textActivated(const QString& arg1);

    void on_comboBoxRotatingObjectMaterial_textActivated(const QString& arg1);

    void on_buttonVuMeterPeakColor_clicked();

    void on_checkBoxVuMeterPeaks_toggled(bool checked);

    void on_sliderVuMeterPeakHeight_valueChanged(int value);

    void on_sliderScrollerFontXScale_valueChanged(int value);

    void on_sliderScrollerFontYScale_valueChanged(int value);

    void on_sliderScrollerVerticalPosition_valueChanged(int value);

    void on_checkBoxReflectionEnabled_toggled(bool checked);

    void on_checkBoxStarsEnabled_toggled(bool checked);

    void on_buttonReflectionColor_clicked();

    void on_buttonRotatingObjectMaterialColor_clicked();

    void on_buttonRotatingObjectWireframeColor_clicked();

    void on_sliderReflectionOpacity_valueChanged(int value);

    void on_sliderVuMeterWidth_valueChanged(int value);

    void on_sliderVuMeterOpacity_valueChanged(int value);

    void on_sliderPrinterFontXScale_valueChanged(int value);

    void on_sliderPrinterFontYScale_valueChanged(int value);

    void on_checkBoxOnlyOneInstance_toggled(bool checked);

    void on_sliderUadeSilenceTimeOut_valueChanged(int value);

    void on_sliderUadePanning_valueChanged(int value);

    void on_checkBoxUadeSilenceTimeout_toggled(bool checked);

    void on_buttonUadeSonglengthsBrowse_clicked();

    void on_buttonAppearanceSelectionColor_clicked();

    void loadBitmapFont(QString);
    void loadBitmapFontPrinter(QString);

    void on_checkBoxMilliseconds_toggled(bool checked);

    void on_checkBoxAdPlugContinuousPlayback_toggled();

    void on_checkBoxHivelyTrackerContinuousPlayback_toggled();

    void on_checkBoxLibvgmContinuousPlayback_toggled();

    void on_checkBoxSndhPlayerContinuousPlayback_toggled();

    void on_checkBoxVgmstreamContinuousPlayback_toggled();

    void on_checkBoxFmodSeamlessLoop_toggled();

    void on_sliderLibopenmptStereoSeparation_valueChanged(int value);

    void on_checkBoxLibopenmptContinuousPlayback_toggled();

    void on_checkBoxEnqueueItems_toggled(bool checked);

    void on_checkBoxShowLoopPoints_toggled(bool checked) const;

    void on_checkBoxVuMeterEnabled_toggled(bool checked);

    void on_checkBoxScrollerEnabled_toggled(bool checked);

    void on_checkBoxPrinterEnabled_toggled(bool checked);

    void on_sliderStarfieldAmount_valueChanged(int value);

    void on_sliderStarfieldSpeed_valueChanged(int value);

    void on_comboBoxStarfieldDirection_textActivated(const QString& arg1);

    void on_checkBoxScrollerSinusFontScaling_toggled(bool checked);

    void on_buttonVisualizerBackgroundColor_clicked();

    void on_buttonScrollerFontImage_clicked();

    void on_buttonPrinterFontImage_clicked();

    void on_sliderVisualizerResolutionWidth_valueChanged(int value);

    void on_sliderVisualizerResolutionHeight_valueChanged(int value);

    void on_checkBoxVisualizerMaintainAspectRatio_toggled(bool checked);

    void on_comboBoxLibsidplayfpHvscSonglengthsUpdate_textActivated(const QString& arg1);

    void on_buttonLibsidplayfpHvscSonglengthsDownload_clicked();

    void on_sliderRasterBarsAmount_valueChanged(int value);

    void on_sliderRasterBarsVerticalSpacing_valueChanged(int value);

    void on_sliderRasterBarsHeight_valueChanged(int value);

    void on_sliderRasterBarsSpeed_valueChanged(int value);

    void on_checkBoxRasterBarsEnabled_toggled(bool checked);

    void on_checkBoxRotatingObjectEnabled_toggled(bool checked);

    void on_checkBoxRotatingObjectOrbit_toggled(bool checked);

    void on_checkBoxRotatingObjectWireframeEnabled_toggled(bool checked);

    void on_checkBoxScrollerCustomTextEnabled_toggled(bool checked);

    void on_sliderRasterBarsOpacity_valueChanged(int value);

    void on_checkBoxUadeContinuousPlayback_toggled();

    void on_checkBoxUadeSongLengths_toggled(bool checked);

    void on_checkBoxLibopenmptAmigaResampler_toggled(bool checked);

    void on_checkBoxLibxmpContinuousPlayback_toggled();

    void on_checkBoxOnlyOneInstance_clicked();

    void on_checkBoxUadeFilterEmu_toggled(bool checked);

    void on_checkBoxSystray_toggled(bool isChecked);

    void on_checkBoxMinimizeToSystray_toggled(bool isChecked);

    void on_checkBoxMenuBarHidden_toggled(bool isChecked);

    void on_sliderAppearanceNowPlayingFontSize_valueChanged(int value);

    void on_checkBoxLibsidplayfpHvscSonglengthsEnabled_toggled(bool checked);

    void on_checkBoxLibsidplayfpContinuousPlayback_toggled();

private:
    void updateColorButtons();
    void forceUpdateToSliders();
    void updateCheckBoxes();

    void loadAdplugSettings();
    void saveAdplugSettings();
    void loadFmodSettings();
    void saveFmodSettings();
    void loadHivelytrackerSettings();
    void saveHivelytrackerSettings();
    void loadlibopenmptSettings();
    void savelibopenmptSettings();
    void loadLibvgmSettings();
    void saveLibvgmSettings();
    void loadLibxmpSettings();
    void saveLibxmpSettings();
    void loadSidplaySettings();
    void loadSndhPlayerSettings();
    void saveSndhPlayerSettings();
    void loadUadeSettings();
    void saveUadeSettings();
    void loadVgmstreamSettings();
    void saveVgmstreamSettings();

    MainWindow* mainWindow;
    Ui::settingsWindow* ui;
};

#endif // SETTINGSWINDOW_H
