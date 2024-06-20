/**
* 
* @file
*
* @brief UI settings pane implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "interface.h"
#include "interface.ui.h"
#include "playlist/parameters.h"
#include "supp/options.h"
#include "ui/utils.h"
#include "ui/parameters.h"
#include "ui/desktop/language.h"
#include "ui/tools/parameters_helpers.h"
#include "update/parameters.h"
//common includes
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <math/numeric.h>
//qt includes
#include <QtGui/QRadioButton>
//std includes
#include <utility>
//boost includes
#include <boost/range/size.hpp>

namespace
{
  const Parameters::IntType UPDATE_CHECK_PERIODS[] =
  {
    //never
    0,
    //once a day
    86400,
    //once a week
    86400 * 7,
  };
  
  class UpdateCheckPeriodComboboxValue : public Parameters::Integer
  {
  public:
    explicit UpdateCheckPeriodComboboxValue(Parameters::Container::Ptr ctr)
      : Ctr(std::move(ctr))
    {
    }

    int Get() const override
    {
      using namespace Parameters;
      Parameters::IntType val = ZXTuneQT::Update::CHECK_PERIOD_DEFAULT;
      Ctr->FindValue(ZXTuneQT::Update::CHECK_PERIOD, val);
      const Parameters::IntType* const arrPos = std::find(UPDATE_CHECK_PERIODS, std::end(UPDATE_CHECK_PERIODS), val);
      return arrPos != std::end(UPDATE_CHECK_PERIODS)
        ? arrPos - UPDATE_CHECK_PERIODS
        : -1;
    }

    void Set(int val) override
    {
      if (Math::InRange<int>(val, 0, boost::size(UPDATE_CHECK_PERIODS) - 1))
      {
        Ctr->SetValue(Parameters::ZXTuneQT::Update::CHECK_PERIOD, UPDATE_CHECK_PERIODS[val]);
      }
    }

    void Reset() override
    {
      Ctr->RemoveValue(Parameters::ZXTuneQT::Update::CHECK_PERIOD);
    }
  private:
    const Parameters::Container::Ptr Ctr;
  };

  class InterfaceOptionsWidget : public UI::InterfaceSettingsWidget
                               , public UI::Ui_InterfaceSettingsWidget
  {
  public:
    explicit InterfaceOptionsWidget(QWidget& parent)
      : UI::InterfaceSettingsWidget(parent)
      , Options(GlobalOptions::Instance().Get())
      , Language(UI::Language::Create())
    {
      //setup self
      setupUi(this);
      SetupLanguages();

      Require(connect(languageSelect, SIGNAL(currentIndexChanged(int)), SLOT(OnLanguageChanged(int))));
      using namespace Parameters;
      IntegerValue::Bind(*playlistCachedFiles, *Options, ZXTuneQT::Playlist::Cache::FILES_LIMIT, ZXTuneQT::Playlist::Cache::FILES_LIMIT_DEFAULT);
      IntegerValue::Bind(*playlistCacheLimit, *Options, ZXTuneQT::Playlist::Cache::MEMORY_LIMIT_MB, ZXTuneQT::Playlist::Cache::MEMORY_LIMIT_MB_DEFAULT);
      BooleanValue::Bind(*playlistStoreAllProperties, *Options, ZXTuneQT::Playlist::Store::PROPERTIES, ZXTuneQT::Playlist::Store::PROPERTIES_DEFAULT);
      UpdateCheckPeriod = IntegerValue::Bind(*updateCheckPeriod, MakePtr<UpdateCheckPeriodComboboxValue>(Options));
      BooleanValue::Bind(*appSingleInstance, *Options, ZXTuneQT::SINGLE_INSTANCE, ZXTuneQT::SINGLE_INSTANCE_DEFAULT);
      CmdlineTarget = IntegerValue::Bind(*cmdlineTarget, *Options, ZXTuneQT::Playlist::CMDLINE_TARGET, ZXTuneQT::Playlist::CMDLINE_TARGET_DEFAULT);
    }

    void OnLanguageChanged(int idx) override
    {
      const QString lang = languageSelect->itemData(idx).toString();
      Language->Set(lang);
      Options->SetValue(Parameters::ZXTuneQT::UI::LANGUAGE, FromQString(lang));
    }

    //QWidget
    void changeEvent(QEvent* event) override
    {
      if (event && QEvent::LanguageChange == event->type())
      {
        const Parameters::ValueSnapshot blockUpdateCheck(*UpdateCheckPeriod);
        const Parameters::ValueSnapshot blockCmdlineTarget(*CmdlineTarget);
        retranslateUi(this);
      }
      UI::InterfaceSettingsWidget::changeEvent(event);
    }
  private:
    void SetupLanguages()
    {
      const QStringList& langs = Language->GetAvailable();
      for (const auto& id : langs)
      {
        const QString& name(QLocale(id).nativeLanguageName());
        languageSelect->addItem(name, id);
      }
      const QString& curLang = GetCurrentLanguage();
      languageSelect->setCurrentIndex(langs.indexOf(curLang));
    }

    QString GetCurrentLanguage() const
    {
      Parameters::StringType val = FromQString(Language->GetSystem());
      Options->FindValue(Parameters::ZXTuneQT::UI::LANGUAGE, val);
      return ToQString(val);
    }
  private:
    const Parameters::Container::Ptr Options;
    const UI::Language::Ptr Language;
    Parameters::Value* UpdateCheckPeriod;
    Parameters::Value* CmdlineTarget;
  };
}

namespace UI
{
  InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget &parent)
    : QWidget(&parent)
  {
  }

  InterfaceSettingsWidget* InterfaceSettingsWidget::Create(QWidget &parent)
  {
    return new InterfaceOptionsWidget(parent);
  }
}
