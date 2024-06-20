/**
 *
 * @file
 *
 * @brief Asynchronous activity implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "async/src/event.h"
// common includes
#include <make_ptr.h>
#include <pointers.h>
// library includes
#include <async/activity.h>
// std includes
#include <cassert>
#include <thread>

namespace Async
{
  enum class ActivityState
  {
    STOPPED,
    INITIALIZED,
    FAILED,
    STARTED
  };

  class ThreadActivity : public Activity
  {
  public:
    typedef std::shared_ptr<ThreadActivity> Ptr;

    explicit ThreadActivity(Operation::Ptr op)
      : Oper(std::move(op))
      , State(ActivityState::STOPPED)
    {}

    ~ThreadActivity() override
    {
      assert(!IsExecuted() || !"Should call Activity::Wait before stop");
    }

    void Start()
    {
      Thread = std::thread(&ThreadActivity::WorkProc, this);
      if (ActivityState::FAILED == State.WaitForAny(ActivityState::INITIALIZED, ActivityState::FAILED))
      {
        Thread.join();
        State.Set(ActivityState::STOPPED);
        throw LastError;
      }
      State.Set(ActivityState::STARTED);
    }

    bool IsExecuted() const override
    {
      return State.Check(ActivityState::STARTED);
    }

    void Wait() override
    {
      if (Thread.joinable())
      {
        Thread.join();
      }
      ThrowIfError(LastError);
    }

  private:
    void WorkProc()
    {
      LastError = Error();
      try
      {
        Oper->Prepare();
        State.Set(ActivityState::INITIALIZED);
        State.Wait(ActivityState::STARTED);
        Oper->Execute();
        State.Set(ActivityState::STOPPED);
      }
      catch (const Error& err)
      {
        LastError = err;
        State.Set(ActivityState::FAILED);
      }
    }

  private:
    const Operation::Ptr Oper;
    Event<ActivityState> State;
    std::thread Thread;
    Error LastError;
  };

  class StubActivity : public Activity
  {
  public:
    bool IsExecuted() const override
    {
      return false;
    }

    void Wait() override {}
  };
}  // namespace Async

namespace Async
{
  Activity::Ptr Activity::Create(Operation::Ptr operation)
  {
    const ThreadActivity::Ptr result = MakePtr<ThreadActivity>(operation);
    result->Start();
    return result;
  }

  Activity::Ptr Activity::CreateStub()
  {
    static StubActivity stub;
    return MakeSingletonPointer(stub);
  }
}  // namespace Async
