/**
 *
 * @file
 *
 * @brief Display component interface
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#pragma once

// library includes
#include <module/holder.h>
#include <sound/backend.h>
#include <time/duration.h>
// std includes
#include <memory>

// forward declarations
namespace boost
{
  namespace program_options
  {
    class options_description;
  }
}  // namespace boost

class DisplayComponent
{
public:
  typedef std::unique_ptr<DisplayComponent> Ptr;

  virtual ~DisplayComponent() = default;

  // commandline-related part
  virtual const boost::program_options::options_description& GetOptionsDescription() const = 0;

  virtual void Message(const String& msg) = 0;
  virtual void SetModule(Module::Holder::Ptr module, Sound::Backend::Ptr player) = 0;

  // begin frame, returns current position
  virtual Time::AtMillisecond BeginFrame(Sound::PlaybackControl::State state) = 0;
  virtual void EndFrame() = 0;

  static Ptr Create();
};
