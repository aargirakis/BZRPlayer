/**
* 
* @file
*
* @brief Asynchronous adapter for data streams
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <contract.h>
#include <data_streaming.h>
#include <make_ptr.h>
//library includes
#include <async/activity.h>
#include <async/progress.h>
#include <async/sized_queue.h>
//std includes
#include <algorithm>
#include <functional>
#include <list>

namespace Async
{
  template<class T>
  class DataReceiver : public ::DataReceiver<T>
  {
  public:
    DataReceiver(std::size_t workersCount, std::size_t queueSize, typename ::DataReceiver<T>::Ptr delegate)
      : QueueObject(SizedQueue<T>::Create(queueSize))
      , Statistic(Progress::Create())
      , Delegate(std::move(delegate))
    {
      StartAll(workersCount);
    }

    ~DataReceiver() override
    {
      QueueObject->Reset();
      WaitAll();
    }

    void ApplyData(T data) override
    {
      CheckWorkersAvailable();
      Statistic->Produce(1);
      QueueObject->Add(std::move(data));
    }

    void Flush() override
    {
      CheckWorkersAvailable();
      //may not flush queue
      Statistic->WaitForComplete();
      Delegate->Flush();
    }

    static typename ::DataReceiver<T>::Ptr Create(std::size_t workersCount, std::size_t queueSize, typename ::DataReceiver<T>::Ptr delegate)
    {
      return workersCount
        ? MakePtr<DataReceiver>(workersCount, queueSize, delegate)
        : delegate;
    }
  private:
    void StartAll(std::size_t count)
    {
      const typename Operation::Ptr op = MakePtr<TransceiveOperation>(QueueObject, Statistic, Delegate);
      while (Activities.size() < count)
      {
        const Activity::Ptr act = Activity::Create(op);
        Activities.push_back(act);
      }
    }

    std::list<Error> WaitAll()
    {
      std::list<Error> result;
      while (!Activities.empty())
      {
        const Activity::Ptr act = Activities.front();
        try
        {
          act->Wait();
        }
        catch (const Error& err)
        {
          result.push_back(err);
        }
        Activities.pop_front();
      }
      return result;
    }

    void CheckWorkersAvailable()
    {
      if (Activities.end() == std::find_if(Activities.begin(), Activities.end(), std::mem_fn(&Activity::IsExecuted)))
      {
        const auto& errors = WaitAll();
        throw errors.empty()
          ? Error()//TODO
          : *errors.begin();
      }
    }
  private:
    class TransceiveOperation : public Operation
    {
    public:
      TransceiveOperation(typename Queue<T>::Ptr queue, Progress::Ptr stat, typename DataReceiver<T>::Ptr target)
        : QueueObject(std::move(queue))
        , Statistic(std::move(stat))
        , Target(std::move(target))
      {
      }

      void Prepare() override
      {
      }

      void Execute() override
      {
        T val;
        while (QueueObject->Get(val))
        {
          Target->ApplyData(std::move(val));
          Statistic->Consume(1);
        }
      }
    private:
      const typename Queue<T>::Ptr QueueObject;
      const Progress::Ptr Statistic;
      const typename DataReceiver<T>::Ptr Target;
    };
  private:
    const typename Queue<T>::Ptr QueueObject;
    const Progress::Ptr Statistic;
    const typename ::DataReceiver<T>::Ptr Delegate;
    typedef std::list<typename Activity::Ptr> ActivitiesList;
    ActivitiesList Activities;
  };
}
