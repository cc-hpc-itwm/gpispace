// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <drts/scheduler_types.hpp>
#include <drts/private/scheduler_types_implementation.hpp>
#include <sdpa/daemon/Agent.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/GreedyScheduler.hpp>
#include <sdpa/daemon/scheduler/SingleAllocationScheduler.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <sdpa/events/delayed_function_call.hpp>
#include <sdpa/events/put_token.hpp>
#include <sdpa/events/workflow_response.hpp>
#include <sdpa/id_generator.hpp>
#include <sdpa/types.hpp>

#include <fhg/util/macros.hpp>
#include <util-generic/cxx17/holds_alternative.hpp>
#include <util-generic/fallthrough.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <thread>

namespace sdpa
{
  namespace daemon
  {
    namespace
    {
      std::vector<std::string> require_proper_url (std::string url)
      {
        const ::boost::tokenizer<::boost::char_separator<char>> tok
          (url, ::boost::char_separator<char> (":"));

        const std::vector<std::string> vec (tok.begin(), tok.end());

        if (vec.empty() || vec.size() > 2)
        {
          throw std::runtime_error ("configuration of network failed: invalid url: has to be of format 'host[:port]'");
        }

        return vec;
      }

      fhg::com::host_t host_from_url (std::string url)
      {
        return fhg::com::host_t (require_proper_url (url)[0]);
      }
      fhg::com::port_t port_from_url (std::string url)
      {
        const std::vector<std::string> vec (require_proper_url (url));
        if (vec.size() == 2)
        {
          unsigned short port;
          std::istringstream iss (vec[1]);
          iss >> port;
          return fhg::com::port_t {port};
        }

        return fhg::com::port_t {0};
      }
      auto const choose_scheduler =
        []
        ( gspc::scheduler::Type const& scheduler_type
        , std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
            requirements_and_preferences
        , WorkerManager& worker_manager
        , std::mt19937& random_engine
        ) -> std::unique_ptr<sdpa::daemon::Scheduler>
        {
          using namespace gspc::scheduler;
          return fhg::util::visit<std::unique_ptr<sdpa::daemon::Scheduler>>
            ( scheduler_type
            , [&] (CostAwareWithWorkStealing const& scheduler)
              {
                return fhg::util::visit<std::unique_ptr<sdpa::daemon::Scheduler>>
                  ( scheduler._->constructed_from()
                  , [&] (CostAwareWithWorkStealing::SingleAllocation)
                    {
                      return std::make_unique<SingleAllocationScheduler>
                        (requirements_and_preferences, worker_manager);
                    }
                  , [&] (CostAwareWithWorkStealing::CoallocationWithBackfilling)
                    {
                      return std::make_unique<CoallocationScheduler>
                        (requirements_and_preferences, worker_manager);
                    }
                  );
              }
            , [&] (GreedyWithWorkStealing const&)
              {
                return std::make_unique<GreedyScheduler>
                  (requirements_and_preferences, worker_manager, random_engine);
              }
            );
        };
      }

    Agent::Agent
        ( std::string name
        , std::string url
        , std::unique_ptr<::boost::asio::io_service> peer_io_service
        , ::boost::optional<::boost::filesystem::path> const& vmem_socket
        , fhg::com::Certificates const& certificates
        )
      : _name (name)
      , _subscriptions()
      , _job_map_mutex()
      , job_map_()
      , _cleanup_job_map_on_dtor_helper (job_map_)
      , _worker_manager()
      , _cancel_mutex()
      , _scheduling_requested_guard()
      , _scheduling_requested_condition()
      , _scheduling_requested (false)
      , _random_extraction_engine (std::random_device()())
      , mtx_subscriber_()
      , _log_emitter()
      , _event_queue()
      , _network_strategy ( [this] ( fhg::com::p2p::address_t const& source
                                   , events::SDPAEvent::Ptr const& e
                                   )
                          {
                            _event_queue.put (source, e);
                          }
                          , std::move (peer_io_service)
                          , host_from_url (url)
                          , port_from_url (url)
                          , certificates
                          )
      , _workflow_engine
          ( std::bind (&Agent::submit, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&Agent::cancel, this, std::placeholders::_1)
          , std::bind (&Agent::finished, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&Agent::failed, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&Agent::canceled, this, std::placeholders::_1)
          , std::bind (&Agent::token_put, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&Agent::workflow_response_response, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&Agent::gen_id, this)
          , _random_extraction_engine
          )
      , _scheduling_thread (&Agent::scheduling_thread, this)
      , _interrupt_scheduling_thread
          ( [this]
            {
              std::lock_guard<std::mutex> const _ (_scheduling_requested_guard);
              _scheduling_interrupted = true;
              _scheduling_requested_condition.notify_one();
            }
          )
      , _virtual_memory_api
        ( vmem_socket
        ? std::make_unique<iml::Client> (*vmem_socket)
        : nullptr
        )
      , _event_handler_thread (&Agent::handle_events, this)
      , _interrupt_event_queue (_event_queue)
    {}

    Agent::cleanup_job_map_on_dtor_helper::cleanup_job_map_on_dtor_helper
        (job_map_t& m)
      : _ (m)
    {}

    Agent::cleanup_job_map_on_dtor_helper::~cleanup_job_map_on_dtor_helper()
    {
      for (const Job* const pJob : _ | ::boost::adaptors::map_values )
      {
        delete pJob;
      }
    }

    std::string const& Agent::name() const
    {
      return _name;
    }

    ::boost::asio::ip::tcp::endpoint Agent::peer_local_endpoint() const
    {
      return _network_strategy.local_endpoint();
    }

    void Agent::serveJob
      ( WorkerSet const& workers
      , Implementation const& implementation
      , job_id_t const& jobId
      , std::function<fhg::com::p2p::address_t (worker_id_t const&)> address
      )
    {
      Job const* const ptrJob = findJob (jobId);
      if (ptrJob)
      {
        for (auto const& worker : workers)
        {
          child_proxy
            ( this
            , address (worker)
            )
            .submit_job ( ptrJob->id()
                        , ptrJob->activity()
                        , implementation
                        , workers
                        );
        }
      }
    }

    std::string Agent::gen_id()
    {
      static id_generator generator ("job");
      return generator.next();
    }

    Job* Agent::addJob ( sdpa::job_id_t const& job_id
                               , we::type::Activity activity
                               , job_source source
                               , job_handler handler
                               )
    {
      auto requirements_and_preferences
        (activity.requirements_and_preferences (_virtual_memory_api.get()));

      return addJob ( job_id
                    , std::move (activity)
                    , std::move (source)
                    , std::move (handler)
                    , std::move (requirements_and_preferences)
                    );
    }


    Job* Agent::addJobWithNoPreferences
      ( sdpa::job_id_t const& job_id
      , we::type::Activity activity
      , job_source source
      , job_handler handler
      )
    {
      return addJob
        ( job_id
        , std::move (activity)
        , std::move (source)
        , std::move (handler)
        , {{}, {}, null_transfer_cost, 1.0, 0, {}} //empty preferences
        );
    }

    Job* Agent::addJob
      ( sdpa::job_id_t const& job_id
      , we::type::Activity activity
      , job_source source
      , job_handler handler
      , Requirements_and_preferences requirements_and_preferences
      )
    {
      Job* pJob = new Job
        ( job_id
        , std::move (activity)
        , std::move (source)
        , std::move (handler)
        , std::move (requirements_and_preferences)
        );

      std::lock_guard<std::mutex> const _ (_job_map_mutex);

      if (!job_map_.emplace (job_id, pJob).second)
      {
        delete pJob;
        throw std::runtime_error ("job with same id already exists");
      }

      return pJob;
    }

    Job* Agent::findJob (sdpa::job_id_t const& job_id ) const
    {
      std::lock_guard<std::mutex> const _ (_job_map_mutex);

      const job_map_t::const_iterator it (job_map_.find( job_id ));
      return it != job_map_.end() ? it->second : nullptr;
    }

    Job* Agent::require_job
      (job_id_t const& id, std::string const& error) const
    {
      Job* job (findJob (id));
      if (!job)
      {
        throw std::logic_error (error);
      }
      return job;
    }

    void Agent::deleteJob (sdpa::job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const _ (_job_map_mutex);

      const job_map_t::const_iterator it (job_map_.find( job_id ));
      if (it == job_map_.end())
      {
        throw std::runtime_error ("deleteJob: job not found");
      }

      delete it->second;
      job_map_.erase (it);
    }

    void Agent::handleDeleteJobEvent
      ( fhg::com::p2p::address_t const& source
      , events::DeleteJobEvent const* event
      )
    {
      Job* const job (require_job (event->job_id(), "delete_job for unknown job"));

      if (!sdpa::status::is_terminal (job->getStatus()))
      {
        throw std::runtime_error
          ("Cannot delete a job which is in a non-terminal state.");
      }

      if (!::boost::get<job_source_client> (&job->source()))
      {
        throw std::invalid_argument
          ("tried deleting a job not submitted by a client");
      }

      deleteJob (job->id());
      parent_proxy (this, source).delete_job_ack (event->job_id());
    }

    void Agent::handleCancelJobEvent
      ( fhg::com::p2p::address_t const& source
      , events::CancelJobEvent const* event
      )
    {
      Job* const job (require_job (event->job_id(), "cancel_job for unknown job"));

      if (job->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if (sdpa::status::is_terminal (job->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (job->getStatus())
          );
      }

      //! \note send immediately an acknowledgment to the component
      // that requested the cancellation if it does not get a
      // notification right after
      if (::boost::get<job_source_wfe> (&job->source())
         || ( ::boost::get<job_source_client> (&job->source())
            && !isSubscriber (source, job->id())
            )
         )
      {
        parent_proxy (this, source).cancel_job_ack (event->job_id());
      }

      if (::boost::get<job_handler_wfe> (&job->handler()))
      {
        if (job->getStatus() == sdpa::status::RUNNING)
        {
          job->CancelJob();
          _workflow_engine.cancel (job->id());
        }
        else
        {
          job_canceled (job);

          deleteJob (job->id());
        }
      }
      else
      {
        cancel_worker_handled_job (job->id());
      }
    }

    void Agent::emit_gantt ( job_id_t const& id
                                   , we::type::Activity const& activity
                                   , NotificationEvent::state_t state
                                   )
    {
      _log_emitter.emit_message
        ( { NotificationEvent ({name()}, id, state, activity).encoded()
          , gantt_log_category
          }
        );
    }

    fhg::logging::endpoint Agent::logger_registration_endpoint() const
    {
      return _log_emitter.local_endpoint();
    }

    fhg::logging::stream_emitter& Agent::log_emitter()
    {
      return _log_emitter;
    }

    bool Agent::workflow_engine_submit (job_id_t job_id, Job* pJob)
    {
      try
      {
        _workflow_engine.submit (job_id, pJob->activity());

        // Should set the workflow_id here, or send it together with the activity
        pJob->Dispatch();

        return true;
      }
      catch (...)
      {
        fhg::util::current_exception_printer const error (": ");
        _log_emitter.emit ( "Exception occurred: " + error.string()
                          + ". Failed to submit the job " + job_id
                          + " to the workflow engine!"
                          , fhg::logging::legacy::category_level_error
                          );

        //! \note: was failed (job_id, error.string()) but wanted to skip
        //! the emission of the gantt
        job_failed (pJob, error.string());

        return false;
      }
    }

    bool Agent::workflow_submission_is_allowed (Job const* pJob)
    {
      if (job_map_.size() <= 1)
      {
        _scheduler.reset();
        return true;
      }

      auto it = std::find_if
        ( job_map_.begin()
        , job_map_.end()
        , [&] (auto const& job) -> bool
          {
            return fhg::util::cxx17::holds_alternative<job_source_client>
                     (job.second->source())
              && ( to_string (job.second->scheduler_type().get())
                 != to_string (pJob->scheduler_type().get())
                 );
          }
        );

      return (it == job_map_.end());
    }

    void Agent::handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , const events::SubmitJobEvent* evt
      )
    {
      events::SubmitJobEvent const& e (*evt);

      // First, check if the job 'job_id' wasn't already submitted!
      if (e.job_id() && findJob(*e.job_id()))
      {
        parent_proxy (this, source).submit_job_ack (*e.job_id());
        return;
      }

      const job_id_t job_id (e.job_id() ? *e.job_id() : job_id_t (gen_id()));

      Job* const pJob ( addJobWithNoPreferences
                          ( job_id
                          , e.activity()
                          , job_source (job_source_client{})
                          , job_handler_wfe()
                          )
                      );

      parent_proxy (this, source).submit_job_ack (job_id);

      std::lock_guard<std::mutex> const _ (_job_map_mutex);
      if (!workflow_submission_is_allowed (pJob))
      {
        std::string error
          ( "Submission not allowed! Running in parallel multiple workflows "
            "requiring different scheduler types is not allowed."
          );

        _log_emitter.emit ( "Error: " + error
                          , fhg::logging::legacy::category_level_error
                          );

        job_failed (pJob, error);

        return;
      }

      if (!_scheduler)
      {
        _scheduler = choose_scheduler
          ( pJob->scheduler_type().get()
          , [this] (job_id_t job_id)
            {
              return findJob (job_id)->requirements_and_preferences();
            }
          , _worker_manager
          , _random_extraction_engine
          );
      }

      // the jobs submitted by clients are always handled by the workflow engine
      if (workflow_engine_submit (job_id, pJob))
      {
        emit_gantt (job_id, pJob->activity(), NotificationEvent::STATE_STARTED);
      }
    }

    void Agent::handleWorkerRegistrationEvent
      ( fhg::com::p2p::address_t const& source
      , const events::WorkerRegistrationEvent* event
      )
    try
    {
      std::lock_guard<std::mutex> const lock_worker_man (_worker_manager._mutex);

      _worker_manager.add_worker
        ( event->name()
        , event->capabilities()
        , event->allocated_shared_memory_size()
        , event->hostname()
        , source
        );

      request_scheduling();

      child_proxy (this, source)
        .worker_registration_response (::boost::none);
    }
    catch (...)
    {
      child_proxy (this, source)
        .worker_registration_response (std::current_exception());
    }

    void Agent::handleErrorEvent
      ( fhg::com::p2p::address_t const& source
      , const events::ErrorEvent* evt
      )
    {
      sdpa::events::ErrorEvent const& error (*evt);

      // if it'a communication error, inspect all jobs and
      // send results if they are in a terminal state

      switch (error.error_code())
      {
        case events::ErrorEvent::SDPA_ENODE_SHUTDOWN:
        {
          unsubscribe (source);

          std::lock_guard<std::mutex> const _ (_cancel_mutex);

          if (_scheduler)
          {
            _scheduler->reschedule_worker_jobs_and_maybe_remove_worker
              ( source
              , [this] (job_id_t const& job)
                {
                  return findJob (job);
                }
              , [this] (fhg::com::p2p::address_t const& addr, job_id_t const& job)
                {
                  return child_proxy ( this
                                     , addr
                                     ).cancel_job (job);
                }
              , [this] (Job* job)
                {
                  job_failed (job, "Number of retries exceeded!");
                }
              );

            request_scheduling();
          }

          break;
        }
        default:
        {
          _log_emitter.emit ( "Unhandled error ("
                            + std::to_string (error.error_code()) + "): "
                            + error.reason()
                            , fhg::logging::legacy::category_level_error
                            );
        }
      }
    }

    void Agent::submit ( we::layer::id_type const& job_id
                               , we::type::Activity activity
                               )
    try
    {
      if (activity.handle_by_workflow_engine())
      {
        workflow_engine_submit
          ( job_id
          , addJobWithNoPreferences
              (job_id, std::move (activity), job_source_wfe(), job_handler_wfe())
          );
      }
      else
      {
        addJob (job_id, std::move (activity), job_source_wfe(), job_handler_worker());

        _scheduler->submit_job (job_id);
        request_scheduling();
      }
    }
    catch (...)
    {
      _workflow_engine.failed
        (job_id, fhg::util::current_exception_printer (": ").string());
    }

    void Agent::cancel (we::layer::id_type const& job_id)
    {
      delay
        (std::bind (&Agent::cancel_worker_handled_job, this, job_id));
    }

    void Agent::cancel_worker_handled_job
      (we::layer::id_type const& job_id)
    {
      std::lock_guard<std::mutex> const _ (_cancel_mutex);

      Job* const pJob (findJob (job_id));
      if (!pJob)
      {
        //! \note Job may have been removed between wfe requesting cancel
        //! and event thread handling this, which is not an error: wfe
        //! correctly handles that situation and expects us to ignore it.
        return;
      }

      bool const cancel_already_requested
        ( pJob->getStatus() == sdpa::status::CANCELING
        || pJob->getStatus() == sdpa::status::CANCELED
        );

      pJob->CancelJob();

      if (!_scheduler->cancel_job
            ( job_id
            , cancel_already_requested
            , [this, &job_id] (fhg::com::p2p::address_t const& addr)
              {
                return Agent::child_proxy (this, addr).cancel_job (job_id);
              }
            )
         )
      {
        job_canceled (pJob);

        if (!::boost::get<job_source_client> (&pJob->source()))
        {
          deleteJob (job_id);
        }
      }
    }

    void Agent::finished ( we::layer::id_type const& id
                         , we::type::Activity const& result
                         )
    {
      delay (std::bind (&Agent::workflow_finished, this, id, result));
    }

    void Agent::workflow_finished ( we::layer::id_type const& id
                                  , we::type::Activity const& result
                                  )
    {
      Job* const pJob (require_job (id, "got finished message for old/unknown Job " + id));

      job_finished (pJob, result);

      //! \note #817: gantt does not support nesting
      if ( fhg::util::visit<bool>
             ( pJob->source()
             , [] (job_source_wfe const&) { return false; }
             , [] (job_source_client const&) { return true; }
             )
         )
      {
        emit_gantt (pJob->id(), pJob->result(), NotificationEvent::STATE_FINISHED);
      }
    }

    void Agent::failed ( we::layer::id_type const& id
                       , std::string const& reason
                       )
    {
      delay (std::bind (&Agent::workflow_failed, this, id, reason));
    }

    void Agent::workflow_failed ( we::layer::id_type const& id
                                , std::string const& reason
                                )
    {
      Job* const pJob (require_job (id, "got failed message for old/unknown Job " + id));

      job_failed (pJob, reason);

      emit_gantt (pJob->id(), pJob->activity(), NotificationEvent::STATE_FAILED);
    }

    void Agent::canceled (we::layer::id_type const& job_id)
    {
      delay (std::bind (&Agent::workflow_canceled, this, job_id));
    }

    void Agent::workflow_canceled (we::layer::id_type const& job_id)
    {
      Job* const pJob (require_job (job_id, "rts_canceled (unknown job)"));

      job_canceled (pJob);

      emit_gantt (pJob->id(), pJob->result(), NotificationEvent::STATE_CANCELED);
    }

    void Agent::handle_job_termination
      ( fhg::com::p2p::address_t const& source
      , Job* job
      , terminal_state const& state
      )
    {
      auto const results
        ( _scheduler->store_individual_result_and_get_final_if_group_finished
            ( source
            , job->id()
            , state
            )
        );

      if (!results)
      {
        return;
      }

      //! \note rescheduled: never tell workflow engine or modify state!
      if ( _scheduler->reschedule_job_if_the_reservation_was_canceled
             (job->id(), job->getStatus())
         )
      {
        if (job->check_and_inc_retry_counter())
        {
          _scheduler->submit_job (job->id());
        }
        else
        {
          job_failed (job, "Number of retries exceeded!");
        }

        request_scheduling();
        return;
      }

      //! \todo instead of ignoring sub-failures and merging error
      //! messages, just pass on results to the user
      if (job->getStatus() == sdpa::status::CANCELING)
      {
        job_canceled (job);
      }
      else
      {
        std::vector<std::string> errors;
        for (auto& result : results->individual_results)
        {
          if ( JobFSM_::s_failed const* as_failed
             = ::boost::get<JobFSM_::s_failed> (&result.second)
             )
          {
            errors.emplace_back (result.first + ": " + as_failed->message);
          }
        }

        if (errors.empty())
        {
          job_finished (job, results->last_success.result);
        }
        else
        {
          job_failed (job, fhg::util::join (errors.begin(), errors.end(), ", ").string());
        }
      }

      _scheduler->release_reservation (job->id());
      request_scheduling();

      if (::boost::get<job_source_wfe> (&job->source()))
      {
        deleteJob (job->id());
      }
    }

    void Agent::handleJobFinishedEvent
      ( fhg::com::p2p::address_t const& source
      , events::JobFinishedEvent const* event
      )
    {
      handle_job_termination
        ( source
        , require_job (event->job_id(), "job_finished for unknown job")
        , JobFSM_::s_finished (event->result())
        );

      child_proxy (this, source).job_finished_ack (event->job_id());
    }

    void Agent::handleJobFailedEvent
      ( fhg::com::p2p::address_t const& source
      , events::JobFailedEvent const* event
      )
    {
      handle_job_termination
        ( source
        , require_job (event->job_id(), "job_failed for unknown job")
        , JobFSM_::s_failed (event->error_message())
        );

      child_proxy (this, source).job_failed_ack (event->job_id());
    }

    void Agent::handleCancelJobAckEvent
      ( fhg::com::p2p::address_t const& source
      , events::CancelJobAckEvent const* event
      )
    {
      handle_job_termination
        ( source
        , require_job (event->job_id(), "cancel_job_ack for unknown job")
        , JobFSM_::s_canceled()
        );
    }

    void Agent::job_finished
      (Job* job, we::type::Activity const& result)
    {
      job->JobFinished (result);

      struct
      {
        void operator() (job_source_wfe const&) const
        {
          _wfe.finished (_job->id(), _job->result());
        }
        void operator() (job_source_client const&) const {}
        Job* _job;
        we::layer& _wfe;
      } visitor = {job, _workflow_engine};
      ::boost::apply_visitor (visitor, job->source());

      notify_subscribers<events::JobFinishedEvent>
        (job->id(), job->id(), job->result());
    }
    void Agent::job_failed (Job* job, std::string const& reason)
    {
      job->JobFailed (reason);

      struct
      {
        void operator() (job_source_wfe const&) const
        {
          _wfe.failed (_job->id(), _job->error_message());
        }
        void operator() (job_source_client const&) const {}
        Job* _job;
        we::layer& _wfe;
      } visitor = {job, _workflow_engine};
      ::boost::apply_visitor (visitor, job->source());

      notify_subscribers<events::JobFailedEvent>
        (job->id(), job->id(), job->error_message());
    }
    void Agent::job_canceled (Job* job)
    {
      job->CancelJobAck();

      struct
      {
        void operator() (job_source_wfe const&) const
        {
          _wfe.canceled (_job->id());
        }
        void operator() (job_source_client const&) const {}
        Job* _job;
        we::layer& _wfe;
      } visitor = {job, _workflow_engine};
      ::boost::apply_visitor (visitor, job->source());

      notify_subscribers<events::CancelJobAckEvent> (job->id(), job->id());
    }

    void Agent::handle_worker_registration_response
      ( fhg::com::p2p::address_t const&
      , sdpa::events::worker_registration_response const* response
      )
    {
      response->get();
    }

    void Agent::handle_events()
    try
    {
      while (true)
      {
        const std::pair<fhg::com::p2p::address_t, events::SDPAEvent::Ptr> event
          (_event_queue.get());
        try
        {
          event.second->handleBy (event.first, this);
        }
        catch (...)
        {
          sendEventToOther<events::ErrorEvent>
            ( event.first
            , events::ErrorEvent::SDPA_EUNKNOWN
            , fhg::util::current_exception_printer (": ").string()
            );
        }
      }
    }
    catch (decltype (_event_queue)::interrupted const&)
    {
    }

    void Agent::delay (std::function<void()> fun)
    {
      _event_queue.put
        ( fhg::com::p2p::address_t()
        , events::SDPAEvent::Ptr (new events::delayed_function_call (fun))
        );
    }

    void Agent::unsubscribe (fhg::com::p2p::address_t const& id)
    {
      std::lock_guard<std::mutex> const _ (mtx_subscriber_);

      _subscriptions.left.erase (id);
    }

    void Agent::handleSubscribeEvent
      ( fhg::com::p2p::address_t const& source
      , const events::SubscribeEvent* pEvt
      )
    {
      job_id_t const& jobId (pEvt->job_id());

      std::lock_guard<std::mutex> const _ (mtx_subscriber_);

      Job const* const pJob (findJob (jobId));
      if (!pJob)
      {
        throw std::runtime_error ( "Could not subscribe for the job" + jobId
                                 + ". The job does not exist!"
                                 );
      }

      sendEventToOther<events::SubscribeAckEvent> (source, jobId);

      // check if the subscribed jobs are already in a terminal state
      const sdpa::status::code status (pJob->getStatus());
      switch (status)
      {
      case sdpa::status::FINISHED:
        {
          sendEventToOther<events::JobFinishedEvent>
            (source, pJob->id(), pJob->result());
        }
        return;

      case sdpa::status::FAILED:
        {
          sendEventToOther<events::JobFailedEvent>
            (source, pJob->id(), pJob->error_message());
        }
        return;

      case sdpa::status::CANCELED:
        {
          sendEventToOther<events::CancelJobAckEvent> (source, pJob->id());
        }
        return;

      case sdpa::status::PENDING:
      case sdpa::status::RUNNING:
      case sdpa::status::CANCELING:
        // store the subscriber and notify it later, when the job is terminated
        _subscriptions.insert ({source, jobId});
        return;
      }

      INVALID_ENUM_VALUE (sdpa::status::code, status);
    }

    bool Agent::isSubscriber
      (fhg::com::p2p::address_t const& agentId, job_id_t const& job)
    {
      std::lock_guard<std::mutex> const _ (mtx_subscriber_);
      return _subscriptions.count
        (decltype (_subscriptions)::value_type (agentId, job));
    }

    /**
     * Event SubmitJobAckEvent
     * Precondition: an acknowledgment event was received from a parent
     * Action: - if the job was found, put the job into the state Running
     *         - move the job from the submitted queue of the worker worker_id, into its
     *           acknowledged queue
     *         - in the case when the worker was not found, trigger an exception and send back
     *           an error message
     * Postcondition: is either into the Running state or inexistent
     */
    void Agent::handleSubmitJobAckEvent
      (fhg::com::p2p::address_t const& source, const events::SubmitJobAckEvent* pEvent)
    {
      // Only, now should be state of the job updated to RUNNING
      // since it was not rejected, no error occurred etc ....
      //find the job ptrJob and call
      Job* const ptrJob = findJob (pEvent->job_id());
      if(!ptrJob)
      {
        _log_emitter.emit ( "job " + pEvent->job_id()
                          + " could not be acknowledged: the job "
                          + pEvent->job_id() + " not found!"
                          , fhg::logging::legacy::category_level_error
                          );

        throw std::runtime_error
          ("Could not acknowledge job: " + pEvent->job_id() + " not found");
      }

      if (ptrJob->getStatus() == sdpa:: status::CANCELING)
        return;

      ptrJob->Dispatch();
      _scheduler->acknowledge_job_sent_to_worker (pEvent->job_id(), source);
    }

    // respond to a worker that the JobFinishedEvent was received
    void Agent::handleJobFinishedAckEvent
      (fhg::com::p2p::address_t const&, const events::JobFinishedAckEvent* pEvt)
    {
      // The result was successfully delivered by the worker and the WE was notified
      // therefore, I can delete the job from the job map
      if (!findJob (pEvt->job_id()))
      {
        throw std::runtime_error ("Couldn't find the job!");
      }

      // delete it from the map when you receive a JobFinishedAckEvent!
      deleteJob (pEvt->job_id());
    }

    // respond to a worker that the JobFailedEvent was received
    void Agent::handleJobFailedAckEvent
      ( fhg::com::p2p::address_t const&
      , const events::JobFailedAckEvent* pEvt
      )
    {
      if (!findJob (pEvt->job_id()))
      {
        throw std::runtime_error ("Couldn't find the job!");
      }

      // delete it from the map when you receive a JobFailedAckEvent!
      deleteJob (pEvt->job_id());
    }

    namespace
    {
      template<typename Map>
        typename Map::mapped_type take (Map& map, typename Map::key_type key)
      {
        typename Map::iterator const it (map.find (key));
        if (it == map.end())
        {
          throw std::runtime_error ("take: key " + key + " not found");
        }

        typename Map::mapped_type const v (std::move (it->second));
        map.erase (it);
        return v;
      }
    }

    void Agent::handle_put_token
      ( fhg::com::p2p::address_t const& source
      , const events::put_token* event
      )
    try
    {
      auto const job_id (event->job_id());
      Job const* const job (findJob (job_id));

      if (!job)
      {
        throw std::runtime_error
          ("unable to put token: " + event->job_id() + " unknown");
      }
      if (job->getStatus() != sdpa::status::RUNNING)
      {
        throw std::runtime_error
          ("unable to put token: " + event->job_id() + " not running");
      }

      fhg::util::visit<void>
        ( job->handler()
        , [&] (job_handler_worker const&)
          {
            _put_token_source.emplace (event->put_token_id(), source);

            _scheduler->notify_submitted_or_acknowledged_workers
              ( job_id
              , [this, &job_id, &event] (fhg::com::p2p::address_t const& addr)
                {
                  return Agent::child_proxy (this, addr)
                    .put_token ( job_id
                               , event->put_token_id()
                               , event->place_name()
                               , event->value()
                               );
                }
              );
          }
        , [&] (job_handler_wfe const&)
          {
            _put_token_source.emplace (event->put_token_id(), source);

            _workflow_engine.put_token ( job_id
                                       , event->put_token_id()
                                       , event->place_name()
                                       , event->value()
                                       );
          }
        );
    }
    catch (...)
    {
      parent_proxy (this, source).put_token_response
        (event->put_token_id(), std::current_exception());
    }

    void Agent::handle_put_token_response
      ( fhg::com::p2p::address_t const&
      , events::put_token_response const* event
      )
    {
      parent_proxy (this, take (_put_token_source, event->put_token_id()))
        .put_token_response (event->put_token_id(), event->exception());
    }

    void Agent::token_put
      ( std::string put_token_id
      , ::boost::optional<std::exception_ptr> error
      )
    {
      delay
        (std::bind (&Agent::token_put_in_workflow, this, put_token_id, error));
    }

    void Agent::token_put_in_workflow
      ( std::string put_token_id
      , ::boost::optional<std::exception_ptr> error
      )
    {
      parent_proxy (this, take (_put_token_source, put_token_id))
        .put_token_response (put_token_id, error);
    }

    void Agent::handle_workflow_response
      ( fhg::com::p2p::address_t const& source
      , const events::workflow_response* event
      )
    try
    {
      auto const job_id (event->job_id());
      Job const* const job (findJob (job_id));

      if (!job)
      {
        throw std::runtime_error
          ( "unable to request workflow response: " + job_id
          + " unknown"
          );
      }
      if (job->getStatus() != sdpa::status::RUNNING)
      {
        throw std::runtime_error
          ( "unable to request workflow response: " + job_id
          + " not running"
          );
      }

      fhg::util::visit<void>
        ( job->handler()
        , [&] (job_handler_worker const&)
          {
            _workflow_response_source.emplace (event->workflow_response_id(), source);

            _scheduler->notify_submitted_or_acknowledged_workers
              ( job_id
              , [this, &job_id, &event] (fhg::com::p2p::address_t const& addr)
                {
                  return Agent::child_proxy (this, addr)
                    .workflow_response ( job_id
                                       , event->workflow_response_id()
                                       , event->place_name()
                                       , event->value()
                                       );
                }
              );
          }
        , [&] (job_handler_wfe const&)
          {
            _workflow_response_source.emplace (event->workflow_response_id(), source);

            _workflow_engine.request_workflow_response ( job_id
                                                       , event->workflow_response_id()
                                                       , event->place_name()
                                                       , event->value()
                                                       );
          }
        );
    }
    catch (...)
    {
      parent_proxy (this, source).workflow_response_response
        (event->workflow_response_id(), std::current_exception());
    }

    void Agent::handle_workflow_response_response
      ( fhg::com::p2p::address_t const&
      , events::workflow_response_response const* event
      )
    {
      parent_proxy (this, take (_workflow_response_source, event->workflow_response_id()))
        .workflow_response_response (event->workflow_response_id(), event->content());
    }
    void Agent::workflow_response_response
      ( std::string workflow_response_id
      , ::boost::variant<std::exception_ptr, pnet::type::value::value_type> result
      )
    {
      delay (std::bind ( &Agent::workflow_engine_workflow_response_response
                       , this
                       , workflow_response_id
                       , result
                       )
            );
    }

    void Agent::workflow_engine_workflow_response_response
      ( std::string workflow_response_id
      , ::boost::variant<std::exception_ptr, pnet::type::value::value_type> result
      )
    {
      parent_proxy (this, take (_workflow_response_source, workflow_response_id))
        .workflow_response_response (workflow_response_id, result);
    }

    void Agent::scheduling_thread()
    {
      for (;;)
      {
        {
          std::unique_lock<std::mutex> lock (_scheduling_requested_guard);
          _scheduling_requested_condition.wait
            ( lock
            , [this]
              {
                return _scheduling_requested || _scheduling_interrupted;
              }
            );

          if (_scheduling_interrupted)
          {
            break;
          }

          _scheduling_requested = false;
        }

        std::lock_guard<std::mutex> const _ (_cancel_mutex);

        if (_scheduler)
        {
          _scheduler->assign_jobs_to_workers();
          _scheduler->start_pending_jobs
            (std::bind ( &Agent::serveJob
                       , this
                       , std::placeholders::_1
                       , std::placeholders::_2
                       , std::placeholders::_3
                       , std::placeholders::_4
                       )
            );
        }
      }
    }

    void Agent::request_scheduling()
    {
      std::lock_guard<std::mutex> const _ (_scheduling_requested_guard);
      _scheduling_requested = true;
      _scheduling_requested_condition.notify_one();
    }

    Agent::child_proxy::child_proxy
        (Agent* that, fhg::com::p2p::address_t const& address)
      : _that (that)
      , _address (address)
    {}

    void Agent::child_proxy::worker_registration_response
      (::boost::optional<std::exception_ptr> error) const
    {
      _that->sendEventToOther<events::worker_registration_response>
        (_address, std::move (error));
    }

    void Agent::child_proxy::submit_job
      ( ::boost::optional<job_id_t> id
      , we::type::Activity activity
      , ::boost::optional<std::string> const& implementation
      , std::set<worker_id_t> const& workers
      ) const
    {
      _that->sendEventToOther<events::SubmitJobEvent>
        (_address, id, std::move (activity), implementation, workers);
    }

    void Agent::child_proxy::cancel_job (job_id_t id) const
    {
      _that->sendEventToOther<events::CancelJobEvent> (_address, id);
    }

    void Agent::child_proxy::job_failed_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::JobFailedAckEvent> (_address, id);
    }

    void Agent::child_proxy::job_finished_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::JobFinishedAckEvent> (_address, id);
    }

    void Agent::child_proxy::put_token
      ( job_id_t job_id
      , std::string put_token_id
      , std::string place_name
      , pnet::type::value::value_type value
      ) const
    {
      _that->sendEventToOther<events::put_token>
        (_address, job_id, put_token_id, place_name, value);
    }

    void Agent::child_proxy::workflow_response
      ( job_id_t job_id
      , std::string workflow_response_id
      , std::string place_name
      , pnet::type::value::value_type value
      ) const
    {
      _that->sendEventToOther<events::workflow_response>
        (_address, job_id, workflow_response_id, place_name, value);
    }

    Agent::parent_proxy::parent_proxy
        (Agent* that, fhg::com::p2p::address_t const& address)
      : _that (that)
      , _address (address)
    {}

    void Agent::parent_proxy::cancel_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::CancelJobAckEvent> (_address, id);
    }

    void Agent::parent_proxy::delete_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::DeleteJobAckEvent> (_address, id);
    }

    void Agent::parent_proxy::submit_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::SubmitJobAckEvent> (_address, id);
    }

    void Agent::parent_proxy::put_token_response
      (std::string put_token_id, ::boost::optional<std::exception_ptr> error) const
    {
      _that->sendEventToOther<events::put_token_response>
        (_address, put_token_id, error);
    }

    void Agent::parent_proxy::workflow_response_response
      ( std::string workflow_response_id
      , ::boost::variant<std::exception_ptr, pnet::type::value::value_type> content
      ) const
    {
      _that->sendEventToOther<events::workflow_response_response>
        (_address, workflow_response_id, content);
    }
  }
}
