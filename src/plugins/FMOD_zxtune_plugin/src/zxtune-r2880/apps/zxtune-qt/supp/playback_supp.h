/**
* 
* @file
*
* @brief Playback support interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "playlist/supp/data.h"
//library includes
#include <core/module_types.h>
#include <sound/backend.h>
//qt includes
#include <QtCore/QThread>

class PlaybackSupport : public QObject
{
  Q_OBJECT
protected:
  explicit PlaybackSupport(QObject& parent);
public:
  static PlaybackSupport* Create(QObject& parent, Parameters::Accessor::Ptr sndOptions);

public slots:
  virtual void SetItem(Playlist::Item::Data::Ptr item) = 0;
  virtual void Play() = 0;
  virtual void Stop() = 0;
  virtual void Pause() = 0;
  virtual void Seek(int frame) = 0;
signals:
  void OnStartModule(Sound::Backend::Ptr, Playlist::Item::Data::Ptr);
  void OnUpdateState();
  void OnPauseModule();
  void OnResumeModule();
  void OnStopModule();
  void OnFinishModule();
  void ErrorOccurred(const Error&);
};
