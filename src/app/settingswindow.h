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
    void setUiLineEditHvscSonglengthTextForcingRelativePaths(const QString& text);

private slots:
    void updateScrollText();

    void downloadHvscSonglengthsComplete();

    void on_sliderResetVolumeToValue_sliderMoved(int position);

    void on_checkBoxResetVolume_toggled(bool checked);

    void on_sliderResetVolumeToValue_valueChanged(int value);

    void on_SliderNormalizerFadeTim_valueChanged(int value);

    void on_SliderNormalizerThreshold_valueChanged(int value);

    void on_SliderNormalizerMaxAmp_valueChanged(int value);

    void on_checkBoxNormalizer_toggled(bool checked);

    void on_checkBoxReverb_toggled(bool checked);


    void on_buttonOK_clicked();


    void on_comboBoxReverb_textActivated(const QString& arg1);


    void on_buttonBrowseHvscSonglengths_clicked();
    void on_buttonGeneral_clicked();

    void on_buttonPlugins_clicked();

    void on_tableWidgetPlugins_itemClicked(QTableWidgetItem* item);

    void on_buttonMainColor_clicked();

    void on_buttonAppearance_clicked();

    void on_buttonMediumColor_clicked();

    void on_buttonBackgroundColor_clicked();

    void on_buttonBehindBackgroundColor_clicked();

    void on_buttonMainTextColor_clicked();

    void on_buttonButtonColor_clicked();

    void on_buttonDimmedTextColor_clicked();

    void on_buttonMainHoverColor_clicked();

    void on_buttonButtonHoverColor_clicked();

    void on_buttonColorsDefault_clicked();

    void on_buttonVisualizer_clicked();

    void on_buttonColorVUMeterTop_clicked();

    void on_buttonColorVUMeterBottom_clicked();

    void on_buttonColorVUMeterMiddle_clicked();

    void on_sliderRowHeight_valueChanged(int value);

    void on_sliderPlaylistsRowHeight_valueChanged(int value);

    void on_sliderAmplitude_valueChanged(int value);

    void on_sliderFrequency_valueChanged(int value);

    void on_sliderSinusSpeed_valueChanged(int value);

    void on_sliderScrollSpeed_valueChanged(int value);


    void on_buttonColorVUMeterPeak_clicked();

    void on_checkBoxPeaks_toggled(bool checked);

    void on_sliderPeakHeight_valueChanged(int value);

    void on_sliderScrollerXScale_valueChanged(int value);

    void on_sliderScrollerYScale_valueChanged(int value);

    void on_sliderVerticalScrollPosition_valueChanged(int value);

    void on_checkBoxReflectionEnabled_toggled(bool checked);

    void on_checkBoxStarsEnabled_toggled(bool checked);

    void on_buttonColorReflection_clicked();

    void on_buttonBrowseFont_clicked();

    void on_sliderReflectionOpacity_valueChanged(int value);

    void on_sliderVUMeterWidth_valueChanged(int value);

    void on_sliderVumeterOpacity_valueChanged(int value);

    void on_buttonBrowseFontPrinter_clicked();

    void on_sliderPrinterXScale_valueChanged(int value);

    void on_sliderPrinterYScale_valueChanged(int value);

    void on_checkBoxOnlyOneInstance_toggled(bool checked);

    void on_sliderSilenceTimeOut_valueChanged(int value);

    void on_checkBoxSilenceTimeout_toggled(bool checked);

    void on_buttonBrowseUADESonglengths_clicked();

    void on_buttonSelectionColor_clicked();

    void loadBitmapFont(QString);
    void loadBitmapFontPrinter(QString);

    void on_checkBoxMilliseconds_toggled(bool checked);

    void on_SliderStereoSeparationOpenMPT_valueChanged(int value);

    void on_checkBoxEnqueueItems_toggled(bool checked);

    void on_checkBoxVUMeterEnabled_toggled(bool checked);

    void on_checkBoxScrollerEnabled_toggled(bool checked);

    void on_checkBoxPrinterEnabled_toggled(bool checked);

    void on_sliderNumberOfStars_valueChanged(int value);

    void on_sliderStarSpeed_valueChanged(int value);

    void on_comboBoxStarsDirection_textActivated(const QString& arg1);

    void on_checkBoxSinusFontScaling_toggled(bool checked);

    void on_buttonColorVisualizerBackground_clicked();

    void on_buttonScrollerFontImage_clicked();

    void on_buttonPrinterFontImage_clicked();

    void on_sliderResolutionWidth_valueChanged(int value);

    void on_sliderResolutionHeight_valueChanged(int value);

    void on_checkBoxAspectRatio_toggled(bool checked);

    void on_comboBoxHvscSonglengthsUpdateFrequency_textActivated(const QString& arg1);

    void on_buttonDownloadHvscSonglengths_clicked();

    void on_sliderNumberOfRasterBars_valueChanged(int value);

    void on_sliderRasterBarsVerticalSpacing_valueChanged(int value);

    void on_sliderRasterBarsBarHeight_valueChanged(int value);

    void on_sliderRasterBarsSpeed_valueChanged(int value);

    void on_checkBoxRasterbarsEnabled_toggled(bool checked);

    void on_checkBoxCustomScrolltextEnabled_toggled(bool checked);

    void on_sliderRasterbarsOpacity_valueChanged(int value);

    void on_checkBoxSongLengthUADE_toggled(bool checked);

    void on_checkBoxFilterOpenMPT_toggled(bool checked);


    void on_checkBoxOnlyOneInstance_clicked();

    void on_checkBoxFilterEnabled_toggled(bool checked);

    void on_checkBoxSystrayOnQuit_toggled(bool checked);

    void on_sliderNowPlayingFontSize_valueChanged(int value);

    void on_checkBoxHvscSonglengthsEnabled_toggled(bool checked);

private:
    void updateColorButtons();


    void loadSidplaySettings();
    void saveUADESettings();
    void savelibopenmptSettings();
    void loadUADESettings();
    void loadlibopenmptSettings();
    void forceUpdateToSliders();
    void updateCheckBoxes();


    MainWindow* mainWindow;
    Ui::settingsWindow* ui;
};

#endif // SETTINGSWINDOW_H
