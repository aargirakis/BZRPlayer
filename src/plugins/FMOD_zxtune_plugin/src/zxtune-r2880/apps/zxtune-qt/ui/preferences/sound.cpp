/**
* 
* @file
*
* @brief Sound settings pane implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "sound.h"
#include "sound.ui.h"
#include "sound_alsa.h"
#include "sound_dsound.h"
#include "sound_oss.h"
#include "sound_sdl.h"
#include "sound_win32.h"
#include "supp/options.h"
#include "ui/utils.h"
#include "ui/tools/parameters_helpers.h"
//common includes
#include <contract.h>
//library includes
#include <math/numeric.h>
#include <sound/backend_attrs.h>
#include <sound/backends_parameters.h>
#include <sound/service.h>
#include <sound/sound_parameters.h>
#include <strings/array.h>
//boost includes
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/end.hpp>

namespace
{
  static const uint_t FREQUENCES[] =
  {
    8000,
    12000,
    16000,
    22000,
    24000,
    32000,
    44100,
    48000
  };

  Strings::Array GetSystemBackends(Parameters::Accessor::Ptr params)
  {
    return Sound::CreateSystemService(params)->GetAvailableBackends();
  }

  class SoundOptionsWidget : public UI::SoundSettingsWidget
                           , public UI::Ui_SoundSettingsWidget
  {
  public:
    explicit SoundOptionsWidget(QWidget& parent)
      : UI::SoundSettingsWidget(parent)
      , Options(GlobalOptions::Instance().Get())
      , Backends(GetSystemBackends(Options))
    {
      //setup self
      setupUi(this);

      FillFrequences();
      FillBackends();

      Parameters::IntegerValue::Bind(*frameDurationValue, *Options, Parameters::ZXTune::Sound::FRAMEDURATION, Parameters::ZXTune::Sound::FRAMEDURATION_DEFAULT);
      Parameters::IntType freq = Parameters::ZXTune::Sound::FREQUENCY_DEFAULT;
      Options->FindValue(Parameters::ZXTune::Sound::FREQUENCY, freq);
      SetFrequency(freq);
      connect(soundFrequencyValue, SIGNAL(currentIndexChanged(int)), SLOT(ChangeSoundFrequency(int)));

      connect(backendsList, SIGNAL(currentRowChanged(int)), SLOT(SelectBackend(int)));
      connect(moveUp, SIGNAL(released()), SLOT(MoveBackendUp()));
      connect(moveDown, SIGNAL(released()), SLOT(MoveBackendDown()));
    }

    virtual void ChangeSoundFrequency(int idx)
    {
      const qlonglong val = FREQUENCES[idx];
      Options->SetValue(Parameters::ZXTune::Sound::FREQUENCY, val);
    }
    
    virtual void SelectBackend(int idx)
    {
      const String id = Backends[idx];
      for (std::map<String, QWidget*>::const_iterator it = SetupPages.begin(), lim = SetupPages.end(); it != lim; ++it)
      {
        it->second->setVisible(it->first == id);
      }
      settingsHint->setVisible(0 == SetupPages.count(id));
    }
    
    virtual void MoveBackendUp()
    {
      if (const int row = backendsList->currentRow())
      {
        SwapItems(row, row - 1);
        backendsList->setCurrentRow(row - 1);
      }
    }
    
    virtual void MoveBackendDown()
    {
      const int row = backendsList->currentRow();
      if (Math::InRange(row, 0, int(Backends.size() - 2)))
      {
        SwapItems(row, row + 1);
        backendsList->setCurrentRow(row + 1);
      }
    }

    //QWidget
    virtual void changeEvent(QEvent* event)
    {
      if (event && QEvent::LanguageChange == event->type())
      {
        retranslateUi(this);
      }
      UI::SoundSettingsWidget::changeEvent(event);
    }
  private:
    void FillFrequences()
    {
      std::for_each(FREQUENCES, boost::end(FREQUENCES), boost::bind(&SoundOptionsWidget::AddFrequency, this, _1));
    }

    void AddFrequency(uint_t freq)
    {
      soundFrequencyValue->addItem(QString::number(freq));
    }

    void FillBackends()
    {
      std::for_each(Backends.begin(), Backends.end(), boost::bind(&SoundOptionsWidget::AddBackend, this, _1));
      AddPage(&UI::AlsaSettingsWidget::Create);
      AddPage(&UI::DirectSoundSettingsWidget::Create);
      AddPage(&UI::OssSettingsWidget::Create);
      AddPage(&UI::SdlSettingsWidget::Create);
      AddPage(&UI::Win32SettingsWidget::Create);
    }

    void AddPage(UI::BackendSettingsWidget* (*factory)(QWidget&))
    {
      std::auto_ptr<UI::BackendSettingsWidget> wid(factory(*backendGroupBox));
      const String id = wid->GetBackendId();
      if (Backends.end() != std::find(Backends.begin(), Backends.end(), id))
      {
        wid->hide();
        backendGroupLayout->addWidget(wid.get());
        SetupPages[id] = wid.release();
      }
    }
    
    void AddBackend(const String& id)
    {
      backendsList->addItem(ToQString(id));
    }
    
    void SetFrequency(uint_t val)
    {
      const uint_t* const frq = std::find(FREQUENCES, boost::end(FREQUENCES), val);
      if (frq != boost::end(FREQUENCES))
      {
        soundFrequencyValue->setCurrentIndex(frq - FREQUENCES);
      }
    }

    void SaveBackendsOrder()
    {
      static const Char DELIMITER[] = {';', 0};
      const String value = boost::algorithm::join(Backends, DELIMITER);
      Options->SetValue(Parameters::ZXTune::Sound::Backends::ORDER, value);
    }
    
    void SwapItems(int lh, int rh)
    {
      QListWidgetItem* const first = backendsList->item(lh);
      QListWidgetItem* const second = backendsList->item(rh);
      if (first && second)
      {
        const QString firstText = first->text();
        const QString secondText = second->text();
        String& firstId = Backends[lh];
        String& secondId = Backends[rh];
        assert(FromQString(firstText) == firstId);
        assert(FromQString(secondText) == secondId);
        first->setText(secondText);
        second->setText(firstText);
        std::swap(firstId, secondId);
        SaveBackendsOrder();
      }
    }
  private:
    const Parameters::Container::Ptr Options;
    Strings::Array Backends;
    std::map<String, QWidget*> SetupPages;
  };
}
namespace UI
{
  SoundSettingsWidget::SoundSettingsWidget(QWidget& parent)
    : QWidget(&parent)
  {
  }

  SoundSettingsWidget* SoundSettingsWidget::Create(QWidget& parent)
  {
    return new SoundOptionsWidget(parent);
  }
}
