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

  template<typename Function>
    void GreedyScheduler::do_worker_call
      (resource::ID resource_id, job::ID job_id, Function&& function) noexcept
  try
  {
    auto const worker_endpoint
      (_runtime_system.worker_endpoint_for_scheduler (resource_id));

    comm::scheduler::worker::Client client
      (_io_service_for_workers, worker_endpoint);

    std::move (function) (client);
  }
  catch (...)
  {
    finished ( job_id
             , job::finish_reason::WorkerFailure {std::current_exception()}
             );
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

              job::ID const job_id {_next_job_id++, task.id};

              if (!_jobs.emplace (job_id, acquired.requested).second)
              {
                throw std::logic_error ("INCONSISTENCY: Duplicate task id.");
              }

              call_unlocked
                ( lock
                , [&]
                  {
                    do_worker_call
                      ( acquired.requested
                      , job_id
                      , [&] (comm::scheduler::worker::Client& client)
                        {
                          return client.submit
                            ( _comm_server_for_worker.local_endpoint()
                            , job_id
                            , Job {task}
                            );
                        }
                      );
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
            if (!has_finished && !_jobs.empty())
            {
              _injected_or_stopped
                .wait (lock, [&] { return _injected || !!_stopped; });

              if (_injected)
              {
                _injected = false;
              }
            }
            else if (!has_finished && _jobs.empty())
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
            else if (has_finished && _jobs.empty())
            {
              _stopped = true;
            }
            else // if (has_finished && !_jobs.empty())
            {
              throw std::logic_error
                ("INCONSISTENCY: finished while tasks are running.");
            }
          }
        );
    }

    call_unlocked ( lock
                  , [&] (std::unordered_map<job::ID, resource::ID> jobs)
                    {
                      for (auto const& job : jobs)
                      {
                        do_worker_call
                          ( job.second
                          , job.first
                          , [&] (comm::scheduler::worker::Client& client)
                            {
                              return client.cancel (job.first);
                            }
                          );
                      }
                    }
                  , _jobs
                  );

    _injected_or_stopped.wait (lock, [&] { return _jobs.empty(); });
  }

  void GreedyScheduler::finished
    (job::ID job_id, job::FinishReason finish_reason)
  {
    std::lock_guard<std::mutex> const lock (_guard_state);

    auto remove_job
      ( [&]
        {
          _resource_manager.release
            (resource_manager::Trivial::Acquired {_jobs.at (job_id)});

          if (!_jobs.erase (job_id))
          {
            throw std::logic_error ("INCONSISTENCY: finished unknown job");
          }
        }
      );

    fhg::util::visit<void>
      ( finish_reason
      , [&] (job::finish_reason::Finished const& finished)
        {
          auto const& task_result (finished.task_result);

          _workflow_engine.inject (job_id.task_id, task_result);

          remove_job();

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
      , [] (job::finish_reason::WorkerFailure const& failure)
        {
          //! \todo re-schedule? Beware: May be _stopped already!
          throw std::logic_error
            ( "NYI: finished (WorkerFailure): "
            + fhg::util::exception_printer (failure.exception).string()
            );
        }
      , [&] (job::finish_reason::Cancelled const&)
        {
          //! \todo sanity!?
          //! do nothing, just remove job
          remove_job();
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
