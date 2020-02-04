#include <gspc/GreedyScheduler.hpp>

#include <util-generic/make_optional.hpp>

#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <exception>
#include <numeric>
#include <stdexcept>
#include <typeinfo>
#include <utility>

namespace gspc
{
  namespace
  {
    template<typename T, typename U>
      std::vector<T> firsts (std::vector<std::pair<T, U>> const& xs)
    {
      std::vector<T> ys;

      std::transform ( xs.begin(), xs.end()
                     , std::back_inserter (ys)
                     , [] (auto const& x)
                       {
                         return x.first;
                       }
                     );

      return ys;
    }


    Task::SingleResourceWithPreference requirement
      (Task::Requirements const& task_requirements)
    {
      return fhg::util::visit<Task::SingleResourceWithPreference>
        ( task_requirements
        , [&] (Task::SingleResource const& single_resource)
          {
            return Task::SingleResourceWithPreference {single_resource};
          }
        , [&] (Task::SingleResourceWithPreference const&
                single_resource_with_preference
              )
          {
            return single_resource_with_preference;
          }
        , [&] (Task::SingleResourceWithCost
                single_resource_with_cost
              )
          {
            std::sort ( single_resource_with_cost.begin()
                      , single_resource_with_cost.end()
                      , [] (auto const& lhs, auto const& rhs)
                        {
                          return lhs.second < rhs.second;
                        }
                      );

            return firsts (single_resource_with_cost);
          }
        , [] (auto const&) -> Task::SingleResourceWithPreference
          {
            throw std::logic_error ("Unsupport Requirement");
          }
        );
    }
  }

  std::size_t GreedyScheduler::RequirementsHashAndEqByClass::operator()
    (Task::SingleResourceWithPreference const& requirements) const
  {
    auto h (std::hash<resource::Class>{});

    return std::accumulate ( requirements.cbegin(), requirements.cend()
                           , std::size_t {0}
                           , [&] (auto value, auto const& x)
                             {
                               boost::hash_combine (value, h (x.first));
                               return value;
                             }
                           );
  }
  bool GreedyScheduler::RequirementsHashAndEqByClass::operator()
    ( Task::SingleResourceWithPreference const& lhs
    , Task::SingleResourceWithPreference const& rhs
    ) const
  {
    auto l (lhs.cbegin());
    auto r (rhs.cbegin());

    while (l != lhs.cend() && r != rhs.cend())
    {
      if (l->first != r->first)
      {
        return false;
      }

      ++l;
      ++r;
    }

    return l == lhs.cend() && r == rhs.cend();
  }

  GreedyScheduler::Command GreedyScheduler::CommandQueue::get()
  {
    std::unique_lock<std::mutex> lock (_guard);
    _command_added.wait (lock, [this] { return !_commands.empty(); });

    auto command (std::move (_commands.front()));
    _commands.pop_front();
    return command;
  }
  void GreedyScheduler::CommandQueue::put_submit
    ( task::ID task_id
    , resource_manager::WithPreferences::Acquired acquired
    , task::Implementation task_implementation
    )
  {
    std::lock_guard<std::mutex> const lock (_guard);
    return put ( lock
               , Submit { std::move (task_id)
                        , std::move (acquired)
                        , std::move (task_implementation)
                        }
               );
  }
  void GreedyScheduler::CommandQueue::put_failed_to_acquire
    (task::ID task_id, std::exception_ptr error)
  {
    std::lock_guard<std::mutex> const lock (_guard);
    return put ( lock
               , FailedToAcquire {std::move (task_id), std::move (error)}
               );
  }
  void GreedyScheduler::CommandQueue::put_extract()
  {
    std::lock_guard<std::mutex> const lock (_guard);
    if (_commands.empty() || _commands.back().type() != typeid (Extract))
    {
      put (lock, Extract{});
    }
  }
  void GreedyScheduler::CommandQueue::put_stop (std::string reason)
  {
    std::lock_guard<std::mutex> const lock (_guard);
    return put ( lock
               , Stop {std::move (reason)}
               );
  }
  void GreedyScheduler::CommandQueue::put_finished
    (job::ID job_id, task::Result result)
  {
    std::lock_guard<std::mutex> const lock (_guard);
    return put ( lock
               , Finished {std::move (job_id), std::move (result)}
               );
  }
  void GreedyScheduler::CommandQueue::put_cancelled
    (job::ID job_id, task::result::Premature premature_result)
  {
    std::lock_guard<std::mutex> const lock (_guard);
    return put ( lock
               , Cancelled {std::move (job_id), std::move (premature_result)}
               );
  }
  void GreedyScheduler::CommandQueue::put
    (std::lock_guard<std::mutex> const&, Command command)
  {
    _commands.emplace_back (std::move (command));
    _command_added.notify_one();
  }

  GreedyScheduler::GreedyScheduler
      ( comm::scheduler::workflow_engine::Client workflow_engine
      , resource_manager::WithPreferences& resource_manager
      , ScopedRuntimeSystem& runtime_system
      , std::size_t max_attempts
      , boost::optional<std::size_t> max_lookahead
      )
        //! \todo Is it okay to construct the server before
        //! constructing the state?!
    : Scheduler (this)
    , _workflow_engine (workflow_engine)
    , _resource_manager (resource_manager)
    , _runtime_system (runtime_system)
    , _max_attempts (max_attempts)
    , _max_lookahead (max_lookahead)
    , _command_thread
        (fhg::util::bind_this (this, &GreedyScheduler::command_thread))
  {
    _command_queue.put_extract();
  }

  template<typename Function>
    void GreedyScheduler::do_worker_call
      (resource::ID resource_id, Function&& function)
  {
    std::move (function)
      ( _worker_clients.at_or_create
          ( resource_id
          , [&]() -> comm::scheduler::worker::Client
            {
              return
                { _io_service_for_workers
                , _runtime_system.worker_endpoint_for_scheduler (resource_id)
                };
            }
          )
      );
  }

  void GreedyScheduler::schedule_thread
    ( Task::SingleResourceWithPreference requirements
    , std::reference_wrapper<ScheduleQueue> schedule_queue
    )
  try
  {
    auto to_acquire (firsts (requirements));

    while (true)
    {
      auto const interruption_context_and_task_id (schedule_queue.get().pop());
      auto const& interruption_context (interruption_context_and_task_id.first);
      auto const& task_id (interruption_context_and_task_id.second);

      try
      {
        auto const acquired
          (_resource_manager.acquire (interruption_context.get(), to_acquire));

        _command_queue.put_submit
          (task_id, acquired, requirements.at (acquired.selected).second);
      }
      catch (...)
      {
        _command_queue.put_failed_to_acquire
          (task_id, std::current_exception());
      }
    }
  }
  catch (ScheduleQueue::Interrupted const&)
  {
    // do nothing, not responsible for outstanding entries in the
    // schedule_queue
  }

  //! can not be a command, must be atomic in inject
  //! \todo RACE: What if finished before cancel(led), what if
  //! multiple finished, cancel(ed)
  void GreedyScheduler::remove_job (job::ID job_id)
  {
    try
    {
      _resource_manager.release (_jobs.at (job_id));

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

  //! \note not a command to avoid race between finished/cancel (_jobs.at)
  void GreedyScheduler::cancel_job
    (job::ID job_id, task::result::Premature reason)
  {
    try
    {
      do_worker_call
        ( _jobs.at (job_id).requested
        , [&] (comm::scheduler::worker::Client& client)
          {
            return client.cancel (job_id, reason);
          }
        );
    }
    catch (...)
    {
      _command_queue.put_cancelled (job_id, reason);
    }
  }

  void GreedyScheduler::cancel_task
    (task::ID task_id, task::result::Premature reason)
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
    if (schedule_queue_remove (task_id))
    {
      //! no job was created (equiv with FailedToAcquire)
      inject (task_id, reason);
    }
    //! case 3
    else if (_interruption_context_by_task.count (task_id))
    {
      _resource_manager.interrupt (_interruption_context_by_task.at (task_id));
    }
    else
    {
      auto job_id (_job_by_task.find (task_id));

      if (job_id != _job_by_task.end())
      {
        cancel_job (job_id->second, reason);
      }
      // \todo else: mark for cancellation when submit (case 5)
    }
  }

  void GreedyScheduler::inject (task::ID task_id, task::Result task_result)
  {
    auto const inject_result (_workflow_engine.inject (task_id, task_result));

    _failed_attempts.erase (task_id);

    //! \note *each* inject requires an Extract, also the
    //! failures in order to get 'has_finished' (and not block
    //! with no task)
    _command_queue.put_extract();

    std::for_each
      ( inject_result.tasks_with_ignored_result.begin()
      , inject_result.tasks_with_ignored_result.end()
      , [&] (task::ID task_to_cancel_id)
        {
          cancel_task
            (task_to_cancel_id, task::result::CancelIgnored {task_id});
        }
      );
    std::for_each
      ( inject_result.tasks_with_optional_result.begin()
      , inject_result.tasks_with_optional_result.end()
      , [&] (task::ID task_to_cancel_id)
        {
          cancel_task
            (task_to_cancel_id, task::result::CancelOptional {task_id});
        }
      );
  }

  void GreedyScheduler::command_thread()
  {
    while (!_stopping || !_jobs.empty() || _scheduling_items)
    {
      fhg::util::visit<void>
        ( _command_queue.get()
        , [&] (Submit submit)
          {
            task_back_from_schedule_queue (submit.task_id);

            job::ID const job_id {_next_job_id++, submit.task_id};

            if (  !_jobs.emplace (job_id, submit.acquired).second
               || !_job_by_task.emplace (submit.task_id, job_id).second
               )
            {
              throw std::logic_error ("INCONSISTENCY: Duplicate task id.");
            }

            if (_stopping)
            {
              _command_queue.put_cancelled
                ( job_id
                , task::result::Postponed
                    {"Allocated resource but scheduler stopped."}
                );
            }
            else
            {
              try
              {
                do_worker_call
                  ( submit.acquired.requested
                  , [&] (comm::scheduler::worker::Client& client)
                    {
                      return client.submit
                        ( _comm_server_for_worker.local_endpoint()
                        , job_id
                        , Job { _workflow_engine.at (submit.task_id).input
                              , submit.task_implementation
                              }
                        );
                    }
                  );
              }
              catch (...)
              {
                //! \todo better handling, e.g. what if "too many open files"
                //! \todo applies to all catch (...)!?

                //! \note submit might be successful but communication
                //! failed probably leading to a duplicated task
                remove_job (job_id);

                schedule_queue_push (submit.task_id);
              }
            }
          }
        , [&] (FailedToAcquire failed_to_acquire)
          {
            auto const& task_id (failed_to_acquire.task_id);
            auto const& error (failed_to_acquire.error);

            task_back_from_schedule_queue (task_id);

            //! \note is cancelled but without remove_job
            //! no job was created (equiv with cancel_task)
            inject ( task_id
                   , task::result::Postponed
                       {fhg::util::exception_printer (error).string()}
                   );
          }

        , [&] (Extract)
          {
            if (_stopping)
            {
              return;
            }

            fhg::util::visit<void>
              ( _workflow_engine.extract()
              , [&] (task::ID task_id)
                {
                  schedule_queue_push (task_id);

                  if (!_max_lookahead || _scheduling_items < *_max_lookahead)
                  {
                    _command_queue.put_extract();
                  }
                  else
                  {
                    _delayed_extract = true;
                  }
                }
              , [&] (bool has_finished)
                {
                  if (has_finished)
                  {
                    assert (_jobs.empty());

                    stop_with_reason
                      ("INCONSISTENCY: Finished with outstanding jobs.");
                  }
                }
              );
          }

        , [&] (Finished finished)
          {
            remove_job (finished.job_id);

            //! \note scheduler policy to stop everything after the
            //! first error, not strictly required
            //! Alternatives:
            //! - ask user
            //! - do nothing but let workflow engine decide
            fhg::util::visit<void>
              ( finished.task_result
              , [&] (task::result::Failure const&)
                {
                  if ( ++_failed_attempts[finished.job_id.task_id]
                     < _max_attempts
                     )
                  {
                    //! \todo tell workflow engine!? -> list<task_result>
                    //! \todo different resource!?
                    schedule_queue_push (finished.job_id.task_id);
                  }
                  else
                  {
                    inject (finished.job_id.task_id, finished.task_result);

                    stop_with_reason
                      ( "scheduler policy: stop after first task with "
                      + std::to_string (_max_attempts)
                      + " failed attempts"
                      );
                  }
                }
              , [&] (auto const&)
                {
                  inject (finished.job_id.task_id, finished.task_result);
                }
              );
          }
        , [&] (Cancelled cancelled)
          {
            //! \todo sanity!?
            remove_job (cancelled.job_id);

            inject (cancelled.job_id.task_id, cancelled.reason);
          }

        , [&] (Stop stop)
          {
            if (_stopping)
            {
              return;
            }

            _stopping = true;

            for (auto const& job : _jobs | boost::adaptors::map_keys)
            {
              cancel_job (job, task::result::Postponed {stop.reason});
            }

            for ( auto& interruption_context
                : _interruption_context_by_task | boost::adaptors::map_values
                )
            {
              _resource_manager.interrupt (interruption_context);
            }
          }
        );
    }

    schedule_queues_interrupt_and_join_threads();
  }

  void GreedyScheduler::finished
    (job::ID job_id, job::FinishReason finish_reason)
  {
    fhg::util::visit<void>
      ( finish_reason
      , [&] (job::finish_reason::Finished finished)
        {
          if (finished.task_result)
          {
            _command_queue.put_finished (job_id, finished.task_result.value());
          }
          else
          {
            _command_queue.put_finished
              ( job_id
              , task::result::Failure {finished.task_result.error()}
              );
          }
        }
      , [&] (job::finish_reason::WorkerFailure failure)
        {
          throw std::logic_error
            ( "NYI: finished (WorkerFailure): "
            + fhg::util::exception_printer (failure.exception).string()
            );
        }
      , [&] (job::finish_reason::Cancelled cancelled)
        {
          _command_queue.put_cancelled (job_id, cancelled.reason);
        }
      );
  }

  GreedyScheduler::~GreedyScheduler()
  {
    stop_with_reason ("premature destruction!?");
    wait();
  }

  void GreedyScheduler::wait()
  {
    //! \todo allow multiple (concurrent) calls
    if (_command_thread.joinable())
    {
      _command_thread.join();
    }
  }

  void GreedyScheduler::stop()
  {
    stop_with_reason ("user request");
  }

  void GreedyScheduler::stop_with_reason (std::string reason)
  {
    _command_queue.put_stop (reason);
  }

  void GreedyScheduler::schedule_queue_push (task::ID task_id)
  {
    auto requirements
      (requirement (_workflow_engine.at (task_id).requirements));

    auto schedule_queue (_schedule_queues.find (requirements));

    if (schedule_queue == _schedule_queues.end())
    {
      schedule_queue =
        _schedule_queues.emplace ( std::piecewise_construct
                                 , std::forward_as_tuple (requirements)
                                 , std::forward_as_tuple ()
                                 ).first;

      std::reference_wrapper<ScheduleQueue> schedule_queue_reference
        (schedule_queue->second);

      _schedule_threads.emplace
        ( requirements
        , [&, requirements, schedule_queue_reference]
          {
            return schedule_thread (requirements, schedule_queue_reference);
          }
        );
    }

    schedule_queue->second.push
      ( _interruption_context_by_task
        .emplace (task_id, InterruptionContext{}).first->second
      , task_id
      );

    ++_scheduling_items;
  }
  void GreedyScheduler::task_back_from_schedule_queue (task::ID task_id)
  {
    --_scheduling_items;

    _interruption_context_by_task.erase (task_id);

    if (_delayed_extract)
    {
      _command_queue.put_extract();
      _delayed_extract = false;
    }
  }
  bool GreedyScheduler::schedule_queue_remove (task::ID task_id)
  {
    auto requirements
      (requirement (_workflow_engine.at (task_id).requirements));

    if (!_schedule_queues.at (requirements).remove (task_id))
    {
      return false;
    }

    task_back_from_schedule_queue (task_id);

    return true;
  }
  void GreedyScheduler::schedule_queues_interrupt_and_join_threads()
  {
    using boost::adaptors::map_values;

    for (auto& schedule_queue : _schedule_queues | map_values)
    {
      schedule_queue.interrupt();
    }

    for (auto& schedule_thread : _schedule_threads | map_values)
    {
      if (schedule_thread.joinable())
      {
        schedule_thread.join();
      }
    }
  }
}
