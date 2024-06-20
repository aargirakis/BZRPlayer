/**
* 
* @file
*
* @brief Seek controls widget implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "seek_controls.h"
#include "seek_controls.ui.h"
#include "supp/playback_supp.h"
#include "ui/styles.h"
#include "ui/utils.h"
//qt includes
#include <QtGui/QCursor>
#include <QtGui/QToolTip>

namespace
{
  class SeekControlsImpl : public SeekControls
                         , private Ui::SeekControls
  {
  public:
    SeekControlsImpl(QWidget& parent, PlaybackSupport& supp)
      : ::SeekControls(parent)
    {
      //setup self
      setupUi(this);
      timePosition->setRange(0, 0);
      this->connect(timePosition, SIGNAL(sliderReleased()), SLOT(EndSeeking()));

      this->connect(&supp, SIGNAL(OnStartModule(Sound::Backend::Ptr, Playlist::Item::Data::Ptr)),
        SLOT(InitState(Sound::Backend::Ptr, Playlist::Item::Data::Ptr)));
      this->connect(&supp, SIGNAL(OnUpdateState()), SLOT(UpdateState()));
      this->connect(&supp, SIGNAL(OnStopModule()), SLOT(CloseState()));
      supp.connect(this, SIGNAL(OnSeeking(int)), SLOT(Seek(int)));
      timePosition->setStyle(new UI::ClickNGoSliderStyle(*timePosition));
    }

    virtual void InitState(Sound::Backend::Ptr player, Playlist::Item::Data::Ptr item)
    {
      Item = item;
      TrackState = player->GetTrackState();
      timePosition->setRange(0, Item->GetDuration().GetCount());
    }

    virtual void UpdateState()
    {
      if (!isVisible())
      {
        return;
      }
      const uint_t curFrame = TrackState->Frame();
      if (timePosition->isSliderDown())
      {
        ShowTooltip();
      }
      else
      {
        timePosition->setSliderPosition(curFrame);
      }
      timeDisplay->setText(ToQString(GetTimeString(curFrame)));
    }

    virtual void CloseState()
    {
      timePosition->setRange(0, 0);
      timeDisplay->setText(QLatin1String("-:-.-"));
    }

    virtual void EndSeeking()
    {
      OnSeeking(timePosition->value());
    }

    //QWidget
    virtual void changeEvent(QEvent* event)
    {
      if (event && QEvent::LanguageChange == event->type())
      {
        retranslateUi(this);
      }
      ::SeekControls::changeEvent(event);
    }
  private:
    void ShowTooltip()
    {
      const uint_t sliderPos = timePosition->value();
      const QString text = ToQString(GetTimeString(sliderPos));
      const QPoint& mousePos = QCursor::pos();
      QToolTip::showText(mousePos, text, timePosition, QRect());
    }

    String GetTimeString(uint_t frame) const
    {
      Time::MillisecondsDuration duration = Item->GetDuration();
      duration.SetCount(frame);
      return duration.ToString();
    }
  private:
    Playlist::Item::Data::Ptr Item;
    Module::TrackState::Ptr TrackState;
  };
}

SeekControls::SeekControls(QWidget& parent) : QWidget(&parent)
{
}

SeekControls* SeekControls::Create(QWidget& parent, PlaybackSupport& supp)
{
  return new SeekControlsImpl(parent, supp);
}
