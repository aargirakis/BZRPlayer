/**
 *
 * @file
 *
 * @brief Supported formats pane implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "supported_formats.h"
#include "parameters.h"
#include "supp/options.h"
#include "supported_formats.ui.h"
#include "ui/tools/parameters_helpers.h"
#include "ui/utils.h"
// common includes
#include <contract.h>
// library includes
#include <sound/service.h>
// std includes
#include <set>
// qt includes
#include <QtWidgets/QRadioButton>

namespace
{
  class FileBackendsSet
  {
  public:
    explicit FileBackendsSet(Parameters::Accessor::Ptr options)
    {
      const Sound::Service::Ptr service = Sound::CreateFileService(options);
      for (Sound::BackendInformation::Iterator::Ptr backends = service->EnumerateBackends(); backends->IsValid();
           backends->Next())
      {
        const Sound::BackendInformation::Ptr info = backends->Get();
        Ids.insert(std::make_pair(info->Id(), info->Status()));
      }
    }

    Strings::Array GetAvailable() const
    {
      Strings::Array result;
      for (const auto& id : Ids)
      {
        if (!id.second)
        {
          result.push_back(id.first);
        }
      }
      return result;
    }

    Error GetStatus(const String& id) const
    {
      const Id2Status::const_iterator it = Ids.find(id);
      return it != Ids.end() ? it->second : Error();  // TODO
    }

  private:
    typedef std::map<String, Error> Id2Status;
    Id2Status Ids;
  };

  const Char TYPE_WAV[] = {'w', 'a', 'v', 0};
  const Char TYPE_MP3[] = {'m', 'p', '3', 0};
  const Char TYPE_OGG[] = {'o', 'g', 'g', 0};
  const Char TYPE_FLAC[] = {'f', 'l', 'a', 'c', 0};

  class SupportedFormats
    : public UI::SupportedFormatsWidget
    , private Ui::SupportedFormats
  {
  public:
    explicit SupportedFormats(QWidget& parent)
      : UI::SupportedFormatsWidget(parent)
      , Options(GlobalOptions::Instance().Get())
      , Backends(Options)
    {
      // setup self
      setupUi(this);

      Buttons[TYPE_WAV] = selectWAV;
      Buttons[TYPE_MP3] = selectMP3;
      Buttons[TYPE_OGG] = selectOGG;
      Buttons[TYPE_FLAC] = selectFLAC;

      std::for_each(Buttons.begin(), Buttons.end(), [this](auto button) { SetupButton(button); });
      // fixup
      for (const auto& id2b : Buttons)
      {
        if (id2b.second->isChecked())
        {
          selectWAV->setChecked(!id2b.second->isEnabled());
          return;
        }
      }
      selectWAV->setChecked(true);
    }

    String GetSelectedId() const override
    {
      for (const auto& id2b : Buttons)
      {
        if (id2b.second->isChecked())
        {
          return id2b.first;
        }
      }
      return {};
    }

    QString GetDescription() const override
    {
      for (const auto& id2b : Buttons)
      {
        if (id2b.second->isChecked())
        {
          return id2b.second->text();
        }
      }
      return {};
    }

  private:
    typedef std::map<String, QRadioButton*> IdToButton;

    void SetupButton(IdToButton::value_type but)
    {
      Require(connect(but.second, SIGNAL(toggled(bool)), SIGNAL(SettingsChanged())));
      if (const Error status = Backends.GetStatus(but.first))
      {
        but.second->setEnabled(false);
        but.second->setToolTip(ToQString(status.GetText()));
      }
      else
      {
        but.second->setEnabled(true);
      }
      Parameters::ExclusiveValue::Bind(*but.second, *Options, Parameters::ZXTuneQT::UI::Export::TYPE, but.first);
    }

  private:
    const Parameters::Container::Ptr Options;
    const FileBackendsSet Backends;
    IdToButton Buttons;
  };
}  // namespace

namespace UI
{
  SupportedFormatsWidget::SupportedFormatsWidget(QWidget& parent)
    : QWidget(&parent)
  {}

  SupportedFormatsWidget* SupportedFormatsWidget::Create(QWidget& parent)
  {
    return new SupportedFormats(parent);
  }

  Strings::Array SupportedFormatsWidget::GetSoundTypes()
  {
    return FileBackendsSet(GlobalOptions::Instance().Get()).GetAvailable();
  }
}  // namespace UI
