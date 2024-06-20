/**
 *
 * @file
 *
 * @brief Player access interface
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#pragma once

// local includes
#include "storage.h"
// library includes
#include <module/holder.h>
#include <parameters/container.h>
// platform includes
#include <jni.h>

namespace Player
{
  using TimeBase = Time::Millisecond;

  class Control
  {
  public:
    typedef std::shared_ptr<Control> Ptr;
    virtual ~Control() = default;

    virtual Parameters::Modifier& GetParameters() const = 0;

    virtual uint_t GetPosition() const = 0;
    virtual uint_t Analyze(uint_t maxEntries, uint8_t* levels) const = 0;

    virtual bool Render(uint_t samples, int16_t* buffer) = 0;
    virtual void Seek(uint_t frame) = 0;

    virtual uint_t GetPlaybackPerformance() const = 0;
    virtual uint_t GetPlaybackProgress() const = 0;
  };

  typedef ObjectsStorage<Control::Ptr> Storage;

  jobject Create(JNIEnv* env, const Module::Holder& module, uint_t samplerate);

  void InitJni(JNIEnv*);
  void CleanupJni(JNIEnv*);
}  // namespace Player
