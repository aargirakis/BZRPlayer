/**
* 
* @file
*
* @brief OSS settings pane implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "sound_oss.h"
#include "sound_oss.ui.h"
#include "supp/options.h"
#include "ui/tools/parameters_helpers.h"
//common includes
#include <contract.h>
//library includes
#include <sound/backends_parameters.h>
//qt includes
#include <QtGui/QFileDialog>

namespace
{
  class OssOptionsWidget : public UI::OssSettingsWidget
                         , public UI::Ui_OssSettingsWidget
  {
  public:
    explicit OssOptionsWidget(QWidget& parent)
      : UI::OssSettingsWidget(parent)
      , Options(GlobalOptions::Instance().Get())
    {
      //setup self
      setupUi(this);

      connect(selectDevice, SIGNAL(clicked()), SLOT(DeviceSelected()));
      connect(selectMixer, SIGNAL(clicked()), SLOT(MixerSelected()));

      using namespace Parameters::ZXTune::Sound::Backends::Oss;
      Parameters::StringValue::Bind(*device, *Options, DEVICE, DEVICE_DEFAULT);
      Parameters::StringValue::Bind(*mixer, *Options, MIXER, MIXER_DEFAULT);
    }

    virtual Parameters::Container::Ptr GetSettings() const
    {
      //TODO
      return Parameters::Container::Ptr();
    }

    virtual String GetBackendId() const
    {
      static const Char ID[] = {'o', 's', 's', '\0'};
      return ID;
    }

    virtual QString GetDescription() const
    {
      return nameGroup->title();
    }

    virtual void DeviceSelected()
    {
      QString devFile = device->text();
      if (OpenFileDialog(UI::OssSettingsWidget::tr("Select device"), devFile))
      {
        device->setText(devFile);
      }
    }

    virtual void MixerSelected()
    {
      QString mixFile = mixer->text();
      if (OpenFileDialog(UI::OssSettingsWidget::tr("Select mixer"), mixFile))
      {
        mixer->setText(mixFile);
      }
    }

    //QWidget
    virtual void changeEvent(QEvent* event)
    {
      if (event && QEvent::LanguageChange == event->type())
      {
        retranslateUi(this);
      }
      UI::OssSettingsWidget::changeEvent(event);
    }
  private:
    bool OpenFileDialog(const QString& title, QString& filename)
    {
      //do not use UI::OpenSingleFileDialog for keeping global settings intact
      QFileDialog dialog(this, title, filename, QLatin1String("*"));
      dialog.setFileMode(QFileDialog::ExistingFile);
      dialog.setReadOnly(true);
      dialog.setOption(QFileDialog::DontUseNativeDialog, true);
      dialog.setOption(QFileDialog::HideNameFilterDetails, true);
      if (QDialog::Accepted == dialog.exec())
      {
        filename = dialog.selectedFiles().front();
        return true;
      }
      return false;
    }

  private:
    const Parameters::Container::Ptr Options;
  };
}
namespace UI
{
  OssSettingsWidget::OssSettingsWidget(QWidget& parent)
    : BackendSettingsWidget(parent)
  {
  }

  BackendSettingsWidget* OssSettingsWidget::Create(QWidget& parent)
  {
    return new OssOptionsWidget(parent);
  }
}
