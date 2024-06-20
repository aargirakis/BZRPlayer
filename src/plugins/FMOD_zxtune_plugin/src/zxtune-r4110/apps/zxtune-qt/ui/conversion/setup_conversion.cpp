/**
* 
* @file
*
* @brief Conversion setup dialog implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "setup_conversion.h"
#include "setup_conversion.ui.h"
#include "filename_template.h"
#include "supported_formats.h"
#include "mp3_settings.h"
#include "ogg_settings.h"
#include "flac_settings.h"
#include "supp/options.h"
#include "ui/state.h"
#include "ui/utils.h"
#include "ui/tools/filedialog.h"
#include "ui/tools/parameters_helpers.h"
//common includes
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <io/api.h>
#include <io/providers_parameters.h>
#include <parameters/merged_accessor.h>
#include <sound/backends_parameters.h>
//boost includes
#include <boost/bind.hpp>
//qt includes
#include <QtCore/QThread>
#include <QtGui/QCloseEvent>
#include <QtGui/QPushButton>

namespace
{
  enum
  {
    TEMPLATE_PAGE = 0,
    FORMAT_PAGE = 1,
    SETTINGS_PAGE = 2
  };

  const uint_t MULTITHREAD_BUFFERS_COUNT = 1000;//20 sec usually

  bool HasMultithreadEnvironment()
  {
    return QThread::idealThreadCount() > 1;
  }
  
  Playlist::Item::Conversion::Options::Ptr CreateOptions(const String& type, const QString& filenameTemplate, Parameters::Accessor::Ptr params)
  {
    return MakePtr<Playlist::Item::Conversion::Options>(type, FromQString(filenameTemplate), params);
  }

  class SetupConversionDialogImpl : public UI::SetupConversionDialog
                                  , private UI::Ui_SetupConversionDialog
  {
  public:
    explicit SetupConversionDialogImpl(QWidget& parent)
      : UI::SetupConversionDialog(parent)
      , Options(GlobalOptions::Instance().Get())
      , TargetTemplate(UI::FilenameTemplateWidget::Create(*this))
      , TargetFormat(UI::SupportedFormatsWidget::Create(*this))
    {
      //setup self
      setupUi(this);
      State = UI::State::Create(*this);
      toolBox->insertItem(TEMPLATE_PAGE, TargetTemplate, QString());
      toolBox->insertItem(FORMAT_PAGE, TargetFormat, QString());

      AddBackendSettingsWidget(&UI::CreateMP3SettingsWidget);
      AddBackendSettingsWidget(&UI::CreateOGGSettingsWidget);
      AddBackendSettingsWidget(&UI::CreateFLACSettingsWidget);

      Require(connect(TargetTemplate, SIGNAL(SettingsChanged()), SLOT(UpdateDescriptions())));
      Require(connect(TargetFormat, SIGNAL(SettingsChanged()), SLOT(UpdateDescriptions())));

      Require(connect(buttonBox, SIGNAL(accepted()), SLOT(accept())));
      Require(connect(buttonBox, SIGNAL(rejected()), SLOT(reject())));

      toolBox->setCurrentIndex(TEMPLATE_PAGE);
      useMultithreading->setEnabled(HasMultithreadEnvironment());
      Parameters::BooleanValue::Bind(*useMultithreading, *Options, Parameters::ZXTune::Sound::Backends::File::BUFFERS, false, MULTITHREAD_BUFFERS_COUNT);

      UpdateDescriptions();
      State->Load();
    }

    Playlist::Item::Conversion::Options::Ptr Execute() override
    {
      if (exec())
      {
        return CreateOptions(TargetFormat->GetSelectedId(), TargetTemplate->GetFilenameTemplate(), GlobalOptions::Instance().GetSnapshot());
      }
      else
      {
        return Playlist::Item::Conversion::Options::Ptr();
      }
    }

    void UpdateDescriptions() override
    {
      UpdateTargetDescription();
      UpdateFormatDescription();
      UpdateSettingsDescription();
    }

    //QWidgets virtuals
    void closeEvent(QCloseEvent* event) override
    {
      State->Save();
      event->accept();
    }
  private:
    void AddBackendSettingsWidget(UI::BackendSettingsWidget* factory(QWidget&))
    {
      QWidget* const settingsWidget = toolBox->widget(SETTINGS_PAGE);
      UI::BackendSettingsWidget* const result = factory(*settingsWidget);
      formatSettingsLayout->addWidget(result);
      Require(connect(result, SIGNAL(SettingsChanged()), SLOT(UpdateDescriptions())));
      BackendSettings[result->GetBackendId()] = result;
    }

    void UpdateTargetDescription()
    {
      const QString templ = TargetTemplate->GetFilenameTemplate();
      toolBox->setItemText(TEMPLATE_PAGE, templ);
      if (QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok))
      {
        okButton->setEnabled(!templ.isEmpty());
      }
    }

    void UpdateFormatDescription()
    {
      toolBox->setItemText(FORMAT_PAGE, TargetFormat->GetDescription());
    }

    void UpdateSettingsDescription()
    {
      const String type = TargetFormat->GetSelectedId();
      std::for_each(BackendSettings.begin(), BackendSettings.end(),
        boost::bind(&QWidget::setVisible, boost::bind(&BackendIdToSettings::value_type::second, _1), false));
      const BackendIdToSettings::const_iterator it = BackendSettings.find(type);
      if (it != BackendSettings.end())
      {
        it->second->setVisible(true);
        toolBox->setItemText(SETTINGS_PAGE, it->second->GetDescription());
        toolBox->setItemEnabled(SETTINGS_PAGE, true);
      }
      else
      {
        toolBox->setItemText(SETTINGS_PAGE, UI::SetupConversionDialog::tr("No options"));
        toolBox->setItemEnabled(SETTINGS_PAGE, false);
      }
    }
  private:
    const Parameters::Container::Ptr Options;
    UI::State::Ptr State;
    UI::FilenameTemplateWidget* const TargetTemplate;
    UI::SupportedFormatsWidget* const TargetFormat;
    typedef std::map<String, UI::BackendSettingsWidget*> BackendIdToSettings;
    BackendIdToSettings BackendSettings;
  };
  
  QString GetDefaultFilename(Playlist::Item::Data::Ptr item)
  {
    const String& filePath = item->GetFilePath();
    const IO::Identifier::Ptr id = IO::ResolveUri(filePath);
    return ToQString(id->Filename());
  }
  
  QString FixExtension(const QString& filename, const QString& extension)
  {
    const int pos = filename.indexOf('.');
    if (pos != -1)
    {
      return filename.left(pos + 1) + extension;
    }
    else
    {
      return filename + '.' + extension;
    }
  }
  
  class Formats
  {
  public:
    explicit Formats(const QString& type)
    {
      AddRawType(type);
      AddSoundTypes();
    }
    
    const QStringList& GetFilters() const
    {
      return Filters;
    }
    
    String GetType(int idx) const
    {
      return Types[idx];
    }
  private:
    void AddRawType(const QString& type)
    {
      Types.push_back("");
      Filters << MakeFilter(type);
    }
    
    void AddSoundTypes()
    {
      const Strings::Array& types = UI::SupportedFormatsWidget::GetSoundTypes();
      for (const auto &type : types)
      {
        Types.push_back(type);
        Filters << MakeFilter(ToQString(type));
      }
    }
    
    static QString MakeFilter(const QString& type)
    {
      return QString::fromAscii("%1 (*.%1)").arg(type);
    }
  private:
    Strings::Array Types;
    QStringList Filters;
  };

  Parameters::Accessor::Ptr CreateSaveAsParameters()
  {
    //force simpliest mode
    const Parameters::Accessor::Ptr base = GlobalOptions::Instance().GetSnapshot();
    const Parameters::Container::Ptr overriden = Parameters::Container::Create();
    overriden->SetValue(Parameters::ZXTune::IO::Providers::File::OVERWRITE_EXISTING, 1);
    overriden->SetValue(Parameters::ZXTune::IO::Providers::File::SANITIZE_NAMES, 0);
    overriden->SetValue(Parameters::ZXTune::IO::Providers::File::CREATE_DIRECTORIES, 0);
    return Parameters::CreateMergedAccessor(overriden, base);
  }
}

namespace UI
{
  SetupConversionDialog::SetupConversionDialog(QWidget& parent)
    : QDialog(&parent)
  {
  }

  SetupConversionDialog::Ptr SetupConversionDialog::Create(QWidget& parent)
  {
    return MakePtr<SetupConversionDialogImpl>(parent);
  }
  
  Playlist::Item::Conversion::Options::Ptr GetExportParameters(QWidget& parent)
  {
    QString nameTemplate;
    if (UI::GetFilenameTemplate(parent, nameTemplate))
    {
      return CreateOptions(String(), nameTemplate, GlobalOptions::Instance().Get());
    }
    else
    {
      return Playlist::Item::Conversion::Options::Ptr();
    }
  }

  Playlist::Item::Conversion::Options::Ptr GetConvertParameters(QWidget& parent)
  {
    const SetupConversionDialog::Ptr dialog = SetupConversionDialog::Create(parent);
    return dialog->Execute();
  }

  Playlist::Item::Conversion::Options::Ptr GetSaveAsParameters(Playlist::Item::Data::Ptr item)
  {
    if (item->GetState())
    {
      return Playlist::Item::Conversion::Options::Ptr();
    }
    const QString type = ToQString(item->GetType()).toLower();
    const Formats formats(type);
    //QFileDialog automatically change extension only when filter is selected
    QString filename = FixExtension(GetDefaultFilename(item), type);
    int typeIndex = 0;
    if (UI::SaveFileDialog(QString(), type, formats.GetFilters(), filename, &typeIndex))
    {
      return CreateOptions(formats.GetType(typeIndex), filename, CreateSaveAsParameters());
    }
    else
    {
      return Playlist::Item::Conversion::Options::Ptr();
    }
  }
}
