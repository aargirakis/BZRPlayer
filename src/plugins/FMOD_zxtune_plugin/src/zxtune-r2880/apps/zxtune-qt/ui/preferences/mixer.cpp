/**
* 
* @file
*
* @brief Single channel mixer widget implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "mixer.h"
#include "mixer.ui.h"
//common includes
#include <contract.h>

namespace
{
  const char* CHANNEL_NAMES[] =
  {
    QT_TRANSLATE_NOOP("UI::MixerWidget", "Left"),
    QT_TRANSLATE_NOOP("UI::MixerWidget", "Right")
  };

  class MixerWidgetImpl : public UI::MixerWidget
                        , private Ui::Mixer
  {
  public:
    MixerWidgetImpl(QWidget& parent, UI::MixerWidget::Channel chan)
      : UI::MixerWidget(parent)
      , Chan(chan)
    {
      //setup self
      setupUi(this);
      LoadName();

      Require(connect(channelValue, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int))));
    }

    virtual void setValue(int val)
    {
      channelValue->setValue(val);
    }

    //QWidget
    virtual void changeEvent(QEvent* event)
    {
      if (event && QEvent::LanguageChange == event->type())
      {
        LoadName();
      }
      UI::MixerWidget::changeEvent(event);
    }
  private:
    void LoadName()
    {
      channelName->setText(UI::MixerWidget::tr(CHANNEL_NAMES[Chan]));
    }
  private:
    const UI::MixerWidget::Channel Chan;
  };
}

namespace UI
{
  MixerWidget::MixerWidget(QWidget& parent)
    : QWidget(&parent)
  {
  }
  
  MixerWidget* MixerWidget::Create(QWidget& parent, MixerWidget::Channel chan)
  {
    return new MixerWidgetImpl(parent, chan);
  }
}

namespace
{
  using namespace Parameters;
  
  class MixerValueImpl : public MixerValue
  {
  public:
    MixerValueImpl(UI::MixerWidget& parent, Parameters::Container& ctr, const Parameters::NameType& name, int defValue)
      : MixerValue(parent)
      , Container(ctr)
      , Name(name)
    {
      Parameters::IntType value = defValue;
      Container.FindValue(Name, value);
      parent.setValue(value);
      Require(connect(&parent, SIGNAL(valueChanged(int)), SLOT(SetValue(int))));
    }

    virtual void SetValue(int value)
    {
      Container.SetValue(Name, value);
    }
  private:
    Parameters::Container& Container;
    const Parameters::NameType Name;
  };
}

namespace Parameters
{
  MixerValue::MixerValue(QObject& parent) : QObject(&parent)
  {
  }

  void MixerValue::Bind(UI::MixerWidget& mix, Container& ctr, const NameType& name, int defValue)
  {
    new MixerValueImpl(mix, ctr, name, defValue);
  }
}
