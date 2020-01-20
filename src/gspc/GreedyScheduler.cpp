#include <gspc/GreedyScheduler.hpp>

#include <gspc/comm/scheduler/worker/Client.hpp>

#include <util-generic/make_optional.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/format.hpp>

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
    , _schedule_thread
        (fhg::util::bind_this (this, &GreedyScheduler::schedule_thread))
    , _command_thread
        (fhg::util::bind_this (this, &GreedyScheduler::command_thread))
  {
    _command_queue.put (Extract{});
  }

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
      (resource::ID resource_id, Function&& function)
  {
    auto const worker_endpoint
      (_runtime_system.worker_endpoint_for_scheduler (resource_id));

    comm::scheduler::worker::Client client
      (_io_service_for_workers, worker_endpoint);

    std::move (function) (client);
  }

  void GreedyScheduler::schedule_thread()
  {
    try
    {
      while (!_stopped)
      {
        auto const task (_schedule_queue.pop().first);

        _command_queue.put
          (Submit {task, _resource_manager.acquire (task.resource_class)});
      }
    }
    catch (ScheduleQueue::Interrupted const&)
    {
      assert (_stopped);
    }
    catch (interface::ResourceManager::Interrupted const&)
    {
      assert (_stopped);
    }
  }

  void GreedyScheduler::command_thread()
  {
    //! can not be a command, must be atomic in inject
    //! \todo RACE: What if finished before cancel(led), what if
    //! multiple finished, cancel(ed)
    auto remove_job
      ( [&] (job::ID job_id)
        {
          try
          {
            _resource_manager.release
              (resource_manager::Trivial::Acquired {_jobs.at (job_id)});

            if (!_jobs.erase (job_id) || !_job_by_task.erase (job_id.task_id))
            {
              throw std::logic_error ("INCONSISTENCY: finished unknown job");
            }
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                  (str (boost::format ("remove job '%1%'") % job_id))
              );
          }

          //! \todo _command_queue.erase (job_id)!?
          //! \todo _command_queue.erase (task_id)!?
        }
      );

    //! \note not a command to avoid race between finished/cancel (_jobs.at)
    auto cancel_job
      ( [&] (job::ID job_id)
        {
          try
          {
            do_worker_call
              ( _jobs.at (job_id)
              , [&] (comm::scheduler::worker::Client& client)
                {
                  return client.cancel (job_id);
                }
              );
          }
          catch (...)
          {
            _command_queue.put (Cancelled {job_id});
          }
        }
      );

    while (! (_stopped && _jobs.empty()))
    {
      fhg::util::visit<void>
        ( _command_queue.get()
        , [&] (Submit submit)
          {
            if (_stopped)
            {
              return;
            }

            auto const& task (submit.task);
            auto const& acquired (submit.acquired);

            job::ID const job_id {_next_job_id++, task.id};

            if (  !_jobs.emplace (job_id, acquired.requested).second
               || !_job_by_task.emplace (task.id, job_id).second
               )
            {
              throw std::logic_error ("INCONSISTENCY: Duplicate task id.");
            }

            try
            {
              do_worker_call
                ( acquired.requested
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
            catch (...)
            {
              //! \note submit might be successful but communication
              //! failed probably leading to a duplicated task
              remove_job (job_id);

              _schedule_queue.push (task, task.id);
            }
          }
        , [&] (Extract)
          {
            if (_stopped)
            {
              return;
            }

            fhg::util::visit<void>
              ( _workflow_engine.extract()
              , [&] (Task task)
                {
                  _schedule_queue.push (task, task.id);
                  _command_queue.put (Extract{});
                }
              , [&] (bool has_finished)
                {
                  if (has_finished)
                  {
                    assert (_jobs.empty());

                    stop();
                  }
                }
              );
          }
        , [&] (CancelAllTasks)
          {
            for (auto const& job : _jobs | boost::adaptors::map_keys)
            {
              cancel_job (job);
            }
          }

        , [&] (Finished finished)
          {
            remove_job (finished.job_id);

            _command_queue.put
              (Inject {finished.job_id.task_id, finished.task_result});

            if (!finished.task_result)
            {
              stop();
            }
          }
        , [&] (Inject inject)
          {
            auto const inject_result
              (_workflow_engine.inject (inject.task_id, inject.task_result));

            //! \note *each* inject requires an Extract, also the
            //! failures in order to get 'has_finished' (and not block
            //! with no task)
            _command_queue.put (Extract{});

            auto cancel_task
              ( [&] (task::ID const& task_id)
                {
                  //! beware: might be in the schedule/command queue
                  //!                                      handled
                  //! 1 : not extracted -> INCONSISTENCY       [-]
                  //! 2 : extracted but in schedule_queue      [x]
                  //!   : extracted, not in schedule_queue
                  //! 3   : in (blocking) acquire              [ ]
                  //!     : in command queue (submit)
                  //! 4     : before this entry                [x]
                  //! 5     : after this entry                 [ ]
                  //! 6   : in command queue (finished)        [x] // worker will ignore cancel

                  //! case 2:
                  if (_schedule_queue.remove (task_id))
                  {
                    // no job was created
                    _command_queue.put
                      ( Inject
                        { task_id
                        , [&]() -> task::Result
                          {
                            throw std::runtime_error
                              (str ( boost::format ("Cancelled: Heureka by %1%")
                                   % inject.task_id
                                   )
                              );
                          }
                        }
                      );
                  }
                  else
                  {
                    auto job_id (_job_by_task.find (task_id));

                    if (job_id != _job_by_task.end())
                    {
                      cancel_job (job_id->second);
                    }
                    // \todo else: mark for cancellation when submit (cases 3 and 5)
                  }
                }
              );

            std::for_each
              ( inject_result.tasks_with_ignored_result.begin()
              , inject_result.tasks_with_ignored_result.end()
              , cancel_task
              );
            std::for_each
              ( inject_result.tasks_with_optional_result.begin()
              , inject_result.tasks_with_optional_result.end()
              , cancel_task
              );
          }
        , [&] (Cancelled cancelled)
          {
            //! \todo sanity!?
            //! do nothing, just remove job
            remove_job (cancelled.job_id);
          }
        );
    }
  }

  void GreedyScheduler::finished
    (job::ID job_id, job::FinishReason finish_reason)
  {
    fhg::util::visit<void>
      ( finish_reason
      , [&] (job::finish_reason::Finished finished)
        {
          _command_queue.put
            (Finished {job_id, std::move (finished.task_result)});
        }
      , [&] (job::finish_reason::WorkerFailure failure)
        {
          throw std::logic_error
            ( "NYI: finished (WorkerFailure): "
            + fhg::util::exception_printer (failure.exception).string()
            );
        }
      , [&] (job::finish_reason::Cancelled)
        {
          _command_queue.put (Cancelled {job_id});
        }
      );
  }

  GreedyScheduler::~GreedyScheduler()
  {
    stop();
    wait();
  }

  void GreedyScheduler::wait()
  {
    //! \todo allow multiple (concurrent) calls
    if (_schedule_thread.joinable())
    {
      _schedule_thread.join();
    }
    if (_command_thread.joinable())
    {
      _command_thread.join();
    }
  }

  void GreedyScheduler::stop()
  {
    _stopped = true;

    _command_queue.put (CancelAllTasks{});

    _resource_manager.interrupt();
    _schedule_queue.interrupt();
  }
}
