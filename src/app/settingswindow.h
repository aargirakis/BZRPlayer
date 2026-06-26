#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "mainwindow.h"

namespace Ui {
    class MainWindow;
    class settingsWindow;
}

class settingsWindow : public QDialog {
    Q_OBJECT

public:
    explicit settingsWindow(QWidget *parent = nullptr);

    ~settingsWindow();

    bool eventFilter(QObject *obj, QEvent *event);

    void changeStyleSheetColor();

    void saveSettingsLibsidplayfp() const;

    void setUiLineEditLibsidplayfpHvscSonglengthsPath(const QString &text) const;

private slots:
    void updateScrollText() const;

    void on_sliderDefaultAudioLevel_sliderMoved(int position);

    void on_checkBoxDefaultAudioLevel_toggled(bool isChecked) const;

    void on_sliderDefaultAudioLevel_valueChanged(int value) const;

    void on_sliderNormalizerFadeTime_valueChanged(int value) const;

    void on_sliderNormalizerThreshold_valueChanged(int value) const;

    void on_sliderNormalizerMaxAmp_valueChanged(int value) const;

    void on_checkBoxNormalizer_toggled(bool isChecked) const;

    void on_checkBoxReverb_toggled(bool isChecked) const;

    void on_buttonOK_clicked();

    void on_comboBoxReverb_textActivated(const QString &arg1) const;

    void on_buttonGeneral_clicked() const;

    void on_buttonPlugins_clicked() const;

    void on_tableWidgetPlugins_itemClicked(QTableWidgetItem *item) const;

    void on_buttonAppearanceMainColor_clicked();

    void on_buttonAppearance_clicked() const;

    void on_buttonAppearanceMediumColor_clicked();

    void on_buttonAppearanceBackgroundColor_clicked();

    void on_buttonAppearanceBehindBackgroundColor_clicked();

    void on_buttonAppearanceMainTextColor_clicked();

    void on_buttonAppearanceButtonColor_clicked();

    void on_buttonAppearanceDimmedTextColor_clicked();

    void on_buttonAppearanceMainHoverColor_clicked();

    void on_buttonAppearanceButtonHoverColor_clicked();

    void on_buttonAppearanceResetColors_clicked() const;

    void on_buttonVisualizer_clicked() const;

    void on_buttonVuMeterTopColor_clicked();

    void on_buttonVuMeterBottomColor_clicked();

    void on_buttonVuMeterMiddleColor_clicked();

    void on_sliderAppearancePlaylistItemRowHeight_valueChanged(int value) const;

    void on_sliderAppearancePlaylistRowHeight_valueChanged(int value) const;

    void on_sliderScrollerAmplitude_valueChanged(int value) const;

    void on_sliderScrollerFrequency_valueChanged(int value) const;

    void on_sliderScrollerSinusSpeed_valueChanged(int value) const;

    void on_sliderScrollerScrollSpeed_valueChanged(int value) const;

    void on_sliderRotatingObjectFocalLength_valueChanged(int value) const;

    void on_sliderRotatingObjectOrbitSize_valueChanged(int value) const;

    void on_sliderRotatingObjectOrbitSpeed_valueChanged(int value) const;

    void on_sliderRotatingObjectModelSize_valueChanged(int value) const;

    void on_comboBoxRotatingObjectModel_textActivated(const QString &arg1) const;

    void on_comboBoxRotatingObjectMaterial_textActivated(const QString &arg1) const;

    void on_buttonVuMeterPeakColor_clicked();

    void on_checkBoxVuMeterPeaks_toggled(bool isChecked) const;

    void on_sliderVuMeterPeakHeight_valueChanged(int value) const;

    void on_sliderScrollerFontXScale_valueChanged(int value) const;

    void on_sliderScrollerFontYScale_valueChanged(int value) const;

    void on_sliderScrollerVerticalPosition_valueChanged(int value) const;

    void on_checkBoxReflectionEnabled_toggled(bool isChecked) const;

    void on_checkBoxStarsEnabled_toggled(bool isChecked) const;

    void on_buttonReflectionColor_clicked();

    void on_buttonRotatingObjectMaterialColor_clicked();

    void on_buttonRotatingObjectWireframeColor_clicked();

    void on_sliderReflectionOpacity_valueChanged(int value) const;

    void on_sliderVuMeterWidth_valueChanged(int value) const;

    void on_sliderVuMeterOpacity_valueChanged(int value) const;

    void on_sliderPrinterFontXScale_valueChanged(int value) const;

    void on_sliderPrinterFontYScale_valueChanged(int value) const;

    void on_checkBoxOnlyOneInstance_toggled(bool isChecked) const;

    void on_buttonAppearanceSelectionColor_clicked();

    void loadBitmapFont(const QString &) const;

    void loadBitmapFontPrinter(const QString &) const;

    void on_checkBoxMilliseconds_toggled(bool isChecked) const;

    void on_checkBoxEnqueueItems_toggled(bool isChecked) const;

    void on_checkBoxShowLoopPoints_toggled(bool isChecked) const;

    void on_checkBoxVuMeterEnabled_toggled(bool isChecked) const;

    void on_checkBoxScrollerEnabled_toggled(bool isChecked) const;

    void on_checkBoxPrinterEnabled_toggled(bool isChecked) const;

    void on_sliderStarfieldAmount_valueChanged(int value) const;

    void on_sliderStarfieldSpeed_valueChanged(int value) const;

    void on_comboBoxStarfieldDirection_textActivated(const QString &arg1) const;

    void on_checkBoxScrollerSinusFontScaling_toggled(bool isChecked) const;

    void on_buttonVisualizerBackgroundColor_clicked();

    void on_buttonScrollerFontImage_clicked() const;

    void on_buttonPrinterFontImage_clicked() const;

    void on_sliderVisualizerResolutionWidth_valueChanged(int value) const;

    void on_sliderVisualizerResolutionHeight_valueChanged(int value) const;

    void on_checkBoxVisualizerMaintainAspectRatio_toggled(bool isChecked) const;

    void on_sliderRasterBarsAmount_valueChanged(int value) const;

    void on_sliderRasterBarsVerticalSpacing_valueChanged(int value) const;

    void on_sliderRasterBarsHeight_valueChanged(int value) const;

    void on_sliderRasterBarsSpeed_valueChanged(int value) const;

    void on_checkBoxRasterBarsEnabled_toggled(bool isChecked) const;

    void on_checkBoxRotatingObjectEnabled_toggled(bool isChecked) const;

    void on_checkBoxRotatingObjectOrbit_toggled(bool isChecked) const;

    void on_checkBoxRotatingObjectWireframeEnabled_toggled(bool isChecked) const;

    void on_checkBoxScrollerCustomTextEnabled_toggled(bool isChecked) const;

    void on_sliderRasterBarsOpacity_valueChanged(int value) const;

    void on_checkBoxOnlyOneInstance_clicked();

    void on_checkBoxSystray_toggled(bool isChecked) const;

    void on_checkBoxMinimizeToSystray_toggled(bool isChecked) const;

    void on_checkBoxMenuBarHidden_toggled(bool isChecked) const;

    void on_sliderAppearanceNowPlayingFontSize_valueChanged(int value) const;

    void on_checkBoxDefaultTrackLength_toggled(bool isChecked) const;

    void on_sliderDefaultTrackLength_valueChanged(int value) const;

    void on_checkBoxAdPlugContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxFmodSeamlessLoop_toggled(bool isChecked) const;

    void on_checkBoxFurnaceContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxHighlyExperimentalContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxHighlyQuixoticContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxHighlyTheoreticalContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxHivelyTrackerContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxLazyusf2ContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxLibopenmptAmigaResampler_toggled(bool isChecked) const;

    void on_checkBoxLibopenmptContinuousPlayback_toggled(bool isChecked) const;

    void on_sliderLibopenmptStereoSeparation_valueChanged(int value) const;

    void on_buttonLibsidplayfpHvscSonglengthsBrowse_clicked();

    void on_buttonLibsidplayfpHvscSonglengthsDownload_clicked();

    void downloadHvscSonglengthsComplete() const;

    void on_checkBoxLibsidplayfpContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxLibsidplayfpHvscSonglengthsEnabled_toggled(bool isChecked) const;

    void on_comboBoxLibsidplayfpHvscSonglengthsUpdate_textActivated(const QString &arg1) const;

    void on_checkBoxLibvgmContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxLibxmpContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxSndhPlayerContinuousPlayback_toggled(bool isChecked) const;

    void on_buttonUadeSonglengthsBrowse_clicked();

    void on_checkBoxUadeContinuousPlayback_toggled(bool isChecked) const;

    void on_checkBoxUadeFilterEmu_toggled(bool isChecked) const;

    void on_checkBoxUadeSilenceTimeout_toggled(bool isChecked) const;

    void on_checkBoxUadeSongLengths_toggled(bool isChecked) const;

    void on_sliderUadePanning_valueChanged(int value) const;

    void on_sliderUadeSilenceTimeOut_valueChanged(int value) const;

    void on_checkBoxVgmstreamContinuousPlayback_toggled(bool isChecked) const;

private:
    void updateColorButtons() const;

    void forceUpdateToSliders() const;

    void updateCheckBoxes() const;

    void loadSettingsAdplug() const;

    void loadSettingsFmod() const;

    void loadSettingsFurnace() const;

    void loadSettingsHighlyExperimental() const;

    void loadSettingsHighlyQuixotic() const;

    void loadSettingsHighlyTheoretical() const;

    void loadSettingsHivelytracker() const;

    void loadSettingsLazyusf2() const;

    void loadSettingsLibopenmpt() const;

    void loadSettingsLibsidplayfp() const;

    void loadSettingsLibvgm() const;

    void loadSettingsLibxmp() const;

    void loadSettingsSndhPlayer() const;

    void loadSettingsUade() const;

    void loadSettingsVgmstream() const;

    void saveSettingsAdplug() const;

    void saveSettingsFmod() const;

    void saveSettingsFurnace() const;

    void saveSettingsHighlyExperimental() const;

    void saveSettingsHighlyQuixotic() const;

    void saveSettingsHighlyTheoretical() const;

    void saveSettingsLazyusf2() const;

    void saveSettingsHivelytracker() const;

    void saveSettingsLibopenmpt() const;

    void saveSettingsLibvgm() const;

    void saveSettingsLibxmp() const;

    void saveSettingsSndhPlayer() const;

    void saveSettingsUade() const;

    void saveSettingsVgmstream() const;

    MainWindow *mainWindow;
    Ui::settingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
