#include "trackerview.h"
#include <plugins.h>
#include "fmod_common.h"
#include "qcoreevent.h"
#include "qevent.h"
#include "soundmanager.h"

TrackerView::TrackerView(QWidget* parent)
    : QOpenGLWidget(parent)
{
    inited = false;
    tracker = new Tracker();
    this->installEventFilter(this);
}

void TrackerView::init()
{
    SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODPATTERN_INFO);
    inited = true;
    tracker->init();
}

Tracker* TrackerView::getTracker()
{
    return tracker;
}

void TrackerView::paintEvent(QPaintEvent* event)
{
    if (tracker == nullptr || !inited || !tracker->m_render || !SoundManager::getInstance().IsPlaying() || (
        SoundManager::getInstance().m_Info1 == nullptr))
    {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), backgroundColor);
        painter.end();
        return;
    }


    if (tracker)
    {
        if ((inited && tracker->m_render && SoundManager::getInstance().IsPlaying()) && (SoundManager::getInstance().
            m_Info1->plugin == PLUGIN_libopenmpt || SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp ||
            SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker || SoundManager::getInstance().m_Info1->
            plugin == PLUGIN_sunvox_lib))
        {
            SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);
            QPainter painter;
            painter.begin(this);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            tracker->paint(&painter, event);
            painter.end();
        }
    }
}

/*!
 Handles changing samples
*/
bool TrackerView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == this)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (event->type() == QEvent::MouseButtonRelease && (mouseEvent->button() & Qt::LeftButton))
        {
            if (tracker->m_render)
            {
                int x = mouseEvent->pos().x() / tracker->m_scale;
                int y = mouseEvent->pos().y() / tracker->m_scale;

                if (x >= tracker->m_trackerview->getbuttonNextSampleX() && x <= (tracker->m_trackerview->
                        getbuttonNextSampleX() + tracker->m_trackerview->getbuttonNextSampleWidth()) && y >= tracker->
                    m_trackerview->getbuttonNextSampleY() && y <= (tracker->m_trackerview->getbuttonNextSampleY() +
                        tracker
                        ->m_trackerview->getbuttonNextSampleHeight()))
                {
                    if (tracker->m_trackerview->getCurrentSample() < tracker->m_info->numSamples - 1)
                    {
                        tracker->m_trackerview->setCurrentSample(tracker->m_trackerview->getCurrentSample() + 1);
                    }
                }
                else if (x >= tracker->m_trackerview->getbuttonPrevSampleX() && x <= (tracker->m_trackerview->
                        getbuttonPrevSampleX() + tracker->m_trackerview->getbuttonPrevSampleWidth()) && y >= tracker->
                    m_trackerview->getbuttonPrevSampleY() && y <= (tracker->m_trackerview->getbuttonPrevSampleY() +
                        tracker
                        ->m_trackerview->getbuttonPrevSampleHeight()))
                {
                    if (tracker->m_trackerview->getCurrentSample() > 0)
                    {
                        tracker->m_trackerview->setCurrentSample(tracker->m_trackerview->getCurrentSample() - 1);
                    }
                }

                if (SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt)
                {
                    //check where we clicked and toggle channels
                    //                    int channel = tracker->m_trackerview->getChannelClicked(mouseEvent->pos().x(),mouseEvent->pos().y());
                    //                    if(channel>=0)
                    //                    {
                    //                        bool enable = this->parent()->getChannelEnabled(channel);
                    //                        this->parent()->setChannelEnabled(channel,!enable);
                    //                    }
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class
        return this->parent()->eventFilter(obj, event);
    }
}

void TrackerView::setBackgroundColor(QColor newColor)
{
    backgroundColor = newColor;
}
