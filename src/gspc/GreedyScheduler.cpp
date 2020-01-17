#include <gspc/GreedyScheduler.hpp>

#include <gspc/comm/scheduler/worker/Client.hpp>

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

namespace gspc
{
  GreedyScheduler::GreedyScheduler
      ( comm::scheduler::workflow_engine::Client workflow_engine
      , resource_manager::Trivial& resource_manager
      , ScopedRuntimeSystem& runtime_system
      )
        //! \todo Is it okay to construct the server before
        //! constructing the state?!
    : Scheduler (this)
    , _workflow_engine (workflow_engine)
    , _resource_manager (resource_manager)
    , _runtime_system (runtime_system)
    , _thread (fhg::util::bind_this ( this
                                    , &GreedyScheduler::scheduling_thread
                                    )
              )
  {}

  template<typename Lock, typename Fun, typename... Args>
    auto call_unlocked (Lock& lock, Fun&& fun, Args&&... args)
      -> decltype (std::forward<Fun> (fun) (std::forward<Args> (args)...))
  {
    FHG_UTIL_FINALLY ([&] { lock.lock(); });
    lock.unlock();
    return std::forward<Fun> (fun) (std::forward<Args> (args)...);
  }

  void GreedyScheduler::scheduling_thread()
  {
    std::unique_lock<std::mutex> lock (_guard_state);

    while (!_stopped)
    {
      fhg::util::visit<void>
        ( _workflow_engine.extract()
        , [&] (Task const& task)
          {
            try
            {
              auto const acquired
                ( call_unlocked
                    ( lock
                    , [&]
                      {
                        return _resource_manager.acquire (task.resource_class);
                      }
                    )
                );

              if (!_tasks.emplace (task.id, acquired.requested).second)
              {
                throw std::logic_error ("INCONSISTENCY: Duplicate task id.");
              }

              call_unlocked
                ( lock
                , [&]
                  {
                    try
                    {
                      auto const worker_endpoint
                        ( _runtime_system.worker_endpoint_for_scheduler
                            (acquired.requested)
                        );
                      //! \todo job_id
                      comm::scheduler::worker::Client ( _io_service_for_workers
                                                      , worker_endpoint
                                                      )
                        . submit ( _comm_server_for_worker.local_endpoint()
                                 , job::ID {0, task.id}
                                 , Job {task}
                                 );
                    }
                    catch (...)
                    {
                      finished ( {0, task.id}
                               , job::finish_reason::WorkerFailure
                                   {std::current_exception()}
                               );
                    }
                  }
                );
            }
            catch (interface::ResourceManager::Interrupted const&)
            {
              assert (_stopped);
            }
          }
        , [&] (bool has_finished)
          {
            if (!has_finished && !_tasks.empty())
            {
              _injected_or_stopped
                .wait (lock, [&] { return _injected || !!_stopped; });

              if (_injected)
              {
                _injected = false;
              }
            }
            else if (!has_finished && _tasks.empty())
            {
              // \todo wait for external event (put_token), go again
              // into extract if not stopped
              _injected_or_stopped
                .wait ( lock
                      , [&] { return /* _put_token || */ !!_stopped; }
                      );

              // if (_put_token)
              // {
              //   _put_token = false;
              // }
            }
            else if (has_finished && _tasks.empty())
            {
              _stopped = true;
            }
            else // if (has_finished && !_tasks.empty())
            {
              throw std::logic_error
                ("INCONSISTENCY: finished while tasks are running.");
            }
          }
        );
    }

    //! \todo
    // for (auto const& task : _tasks)
    // {
    //   _runtime_system.cancel (task);
    // }

    _injected_or_stopped.wait (lock, [&] { return _tasks.empty(); });
  }

  void GreedyScheduler::finished
    (job::ID job_id, job::FinishReason finish_reason)
  {
    auto const task_id (job_id.task_id);

    std::lock_guard<std::mutex> const lock (_guard_state);

    auto remove_task
      ( [&]
        {
          _resource_manager.release
            (resource_manager::Trivial::Acquired {_tasks.at (task_id)});

          if (!_tasks.erase (task_id))
          {
            throw std::logic_error ("INCONSISTENCY: finished unknown tasks");
          }
        }
      );

    fhg::util::visit<void>
      ( finish_reason
      , [&] (job::finish_reason::Finished const& finished)
        {
          auto const& task_result (finished.task_result);

          _workflow_engine.inject (task_id, task_result);

          remove_task();

          if (task_result)
          {
            _injected = true;

            _injected_or_stopped.notify_one();
          }
          else
          {
            stop();
          }
        }
      , [] (job::finish_reason::WorkerFailure const&)
        {
          //! \todo re-schedule? Beware: May be _stopped already!
          throw std::logic_error ("NYI: finished (WorkerFailure)");
        }
      , [&] (job::finish_reason::Cancelled const&)
        {
          //! \todo sanity!?
          //! do nothing, just remove task
          remove_task();
        }
      );
  }

  void GreedyScheduler::wait()
  {
    if (_thread.joinable())
    {
      _thread.join();
    }
  }

  void GreedyScheduler::stop()
  {
    _stopped = true;

    _resource_manager.interrupt();

    _injected_or_stopped.notify_one();
  }
}
