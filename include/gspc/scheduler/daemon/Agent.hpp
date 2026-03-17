// Copyright (C) 2010-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gspc/drts/scheduler_types.hpp>
#include <gspc/scheduler/capability.hpp>
#include <gspc/scheduler/com/NetworkStrategy.hpp>
#include <gspc/scheduler/daemon/Implementation.hpp>
#include <gspc/scheduler/daemon/Job.hpp>
#include <gspc/scheduler/daemon/NotificationEvent.hpp>
#include <gspc/scheduler/daemon/WorkerManager.hpp>
#include <gspc/scheduler/daemon/WorkerSet.hpp>
#include <gspc/scheduler/daemon/scheduler/Scheduler.hpp>
#include <gspc/scheduler/events/CancelJobAckEvent.hpp>
#include <gspc/scheduler/events/DeleteJobAckEvent.hpp>
#include <gspc/scheduler/events/DeleteJobEvent.hpp>
#include <gspc/scheduler/events/EventHandler.hpp>
#include <gspc/scheduler/events/JobFailedAckEvent.hpp>
#include <gspc/scheduler/events/JobFailedEvent.hpp>
#include <gspc/scheduler/events/JobFinishedAckEvent.hpp>
#include <gspc/scheduler/events/JobFinishedEvent.hpp>
#include <gspc/scheduler/events/MgmtEvent.hpp>
#include <gspc/scheduler/events/SchedulerEvent.hpp>
#include <gspc/scheduler/events/SubmitJobAckEvent.hpp>
#include <gspc/scheduler/events/SubmitJobEvent.hpp>
#include <gspc/scheduler/events/SubscribeEvent.hpp>
#include <gspc/scheduler/events/WorkerRegistrationEvent.hpp>
#include <gspc/scheduler/events/worker_registration_response.hpp>
#include <gspc/we/type/requirements_and_preferences.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/iml/stubs.hpp>

#if GSPC_WITH_IML
#include <gspc/iml/Client.hpp>
#endif

#include <gspc/logging/stream_emitter.hpp>

#include <gspc/we/layer.hpp>
#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/schedule_data.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/finally.hpp>
#include <gspc/util/hash/std/pair.hpp>
#include <gspc/util/threadsafe_queue.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <filesystem>
#include <optional>
#include <boost/thread/scoped_thread.hpp>
#include <boost/utility.hpp>

#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <variant>


  namespace gspc::scheduler::daemon {
    class Agent final : public gspc::scheduler::events::EventHandler
    {
    public:
      Agent ( std::string name
            , std::string url
            , std::unique_ptr<::boost::asio::io_service> peer_io_service
            , std::optional<std::filesystem::path> const& vmem_socket
            , gspc::Certificates const& certificates
            );
      ~Agent() override = default;
      Agent (Agent const&) = delete;
      Agent (Agent&&) = delete;
      Agent& operator= (Agent const&) = delete;
      Agent& operator= (Agent&&) = delete;

      std::string const& name() const;
      ::boost::asio::ip::tcp::endpoint peer_local_endpoint() const;
      gspc::logging::endpoint logger_registration_endpoint() const;
      gspc::logging::stream_emitter& log_emitter();

    public:
      // WE interface
      void submit( gspc::we::layer::id_type const& id, gspc::we::type::Activity);
      void cancel (gspc::we::layer::id_type const& id);
      void finished (gspc::we::layer::id_type const& id, gspc::we::type::Activity const& result);
      void failed( gspc::we::layer::id_type const& wfId, std::string const& reason);
      void canceled (gspc::we::layer::id_type const& id);
      void token_put
        (std::string put_token_id, std::optional<std::exception_ptr>);
      void workflow_response_response
        ( std::string workflow_response_id
        , std::variant<std::exception_ptr, gspc::pnet::type::value::value_type>
        );

      void addCapability (Capability const& cpb);
      bool workflow_submission_is_allowed (Job const* pJob);

    private:
      // parents and subscribers
      void unsubscribe (gspc::com::p2p::address_t const&);
      void handleSubscribeEvent (gspc::com::p2p::address_t const& source, const gspc::scheduler::events::SubscribeEvent*) override;

      bool isSubscriber (gspc::com::p2p::address_t const&, job_id_t const&);

      template<typename Event, typename... Args>
        void notify_subscribers (job_id_t job_id, Args&&... args)
      {
        std::lock_guard<std::mutex> const _ (mtx_subscriber_);

        for  ( auto [subscription, end] {_subscriptions.right.equal_range (job_id)}
             ; subscription != end
             ; ++subscription
             )
        {
          sendEventToOther<Event>
            (subscription->second, std::forward<Args> (args)...);
        }

        _subscriptions.right.erase (job_id);
      }

      // event handlers
    private:
      void handleCancelJobEvent
        ( gspc::com::p2p::address_t const&
        , gspc::scheduler::events::CancelJobEvent const*
        ) override;
      void handleDeleteJobEvent
        ( gspc::com::p2p::address_t const&
        , events::DeleteJobEvent const*
        ) override;
      void handleErrorEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::ErrorEvent*
        ) override;
      void handleJobFailedAckEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::JobFailedAckEvent*
        ) override;
      void handleJobFinishedAckEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::JobFinishedAckEvent*
        ) override;
      void handleSubmitJobAckEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::SubmitJobAckEvent*
        ) override;
      void handleSubmitJobEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::SubmitJobEvent*
        ) override;
      void handle_worker_registration_response
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::worker_registration_response*
        ) override;
      void handleWorkerRegistrationEvent
        ( gspc::com::p2p::address_t const&
        , const gspc::scheduler::events::WorkerRegistrationEvent*
        ) override;
      void handle_put_token
        ( gspc::com::p2p::address_t const&
        , const events::put_token*
        ) override;
      void handle_put_token_response
        ( gspc::com::p2p::address_t const&
        , const events::put_token_response*
        ) override;
      void handle_workflow_response
        ( gspc::com::p2p::address_t const&
        , const events::workflow_response*
        ) override;
      void handle_workflow_response_response
        ( gspc::com::p2p::address_t const&
        , events::workflow_response_response const*
        ) override;
      void handleJobFailedEvent
        ( gspc::com::p2p::address_t const&
        , events::JobFailedEvent const*
        ) override;
      void handleJobFinishedEvent
        ( gspc::com::p2p::address_t const&
        , events::JobFinishedEvent const*
        ) override;
      void handleCancelJobAckEvent
        ( gspc::com::p2p::address_t const&
        , events::CancelJobAckEvent const*
        ) override;

      // event communication
      template<typename Event, typename... Args>
        void sendEventToOther
        (gspc::com::p2p::address_t const& address, Args... args)
      {
        _network_strategy.perform<Event> (address, std::forward<Args> (args)...);
      }
      void delay (std::function<void()>);

      // workflow engine
      bool workflow_engine_submit (job_id_t, Job*);

      void handle_job_termination
        (gspc::com::p2p::address_t const&, Job*, terminal_state const&);

      void workflow_finished
        (gspc::we::layer::id_type const&, gspc::we::type::Activity const&);
      void workflow_failed
        (gspc::we::layer::id_type const&, std::string const&);
      void workflow_canceled (gspc::we::layer::id_type const&);
      void token_put_in_workflow
        (std::string, std::optional<std::exception_ptr>);
      void workflow_engine_workflow_response_response
        ( std::string
        , std::variant<std::exception_ptr, gspc::pnet::type::value::value_type>
        );

      //! \todo aggregated results for coallocation jobs and sub jobs
      void job_finished (Job*, gspc::we::type::Activity const&);
      void job_failed (Job*, std::string const& reason);
      void job_canceled (Job*);

      // workers
      void serveJob
        ( WorkerSet const&
        , Implementation const&
        , job_id_t const&
        , std::function<gspc::com::p2p::address_t (worker_id_t const&)>
        );

      // jobs
      std::string gen_id();

      Job* addJob ( gspc::scheduler::job_id_t const& job_id
                  , gspc::we::type::Activity
                  , job_source
                  , job_handler
                  );
      Job* addJob ( gspc::scheduler::job_id_t const& job_id
                  , gspc::we::type::Activity
                  , job_source
                  , job_handler
                  , gspc::we::type::Requirements_and_preferences
                  );
      Job* addJobWithNoPreferences ( gspc::scheduler::job_id_t const&
                                   , gspc::we::type::Activity
                                   , job_source
                                   , job_handler
                                   );

      Job* findJob (gspc::scheduler::job_id_t const& job_id ) const;
      Job* require_job (job_id_t const&, std::string const& error) const;
      void deleteJob (gspc::scheduler::job_id_t const& job_id);

      void cancel_worker_handled_job (gspc::we::layer::id_type const&);

      std::string _name;

      using subscriber_relation_type =
        ::boost::bimap
        < ::boost::bimaps::unordered_multiset_of<gspc::com::p2p::address_t>
        , ::boost::bimaps::unordered_multiset_of<job_id_t>
        , ::boost::bimaps::set_of_relation<>
        >;

      subscriber_relation_type _subscriptions;

      std::unordered_map<std::string, gspc::com::p2p::address_t> _put_token_source;
      std::unordered_map<std::string, gspc::com::p2p::address_t>
        _workflow_response_source;

      using job_map_t = std::unordered_map<gspc::scheduler::job_id_t, gspc::scheduler::daemon::Job*>;

      mutable std::mutex _job_map_mutex;
      job_map_t job_map_;
      struct cleanup_job_map_on_dtor_helper
      {
        cleanup_job_map_on_dtor_helper (Agent::job_map_t&);
        ~cleanup_job_map_on_dtor_helper();
        cleanup_job_map_on_dtor_helper (cleanup_job_map_on_dtor_helper const&) = delete;
        cleanup_job_map_on_dtor_helper (cleanup_job_map_on_dtor_helper&&) = delete;
        cleanup_job_map_on_dtor_helper& operator= (cleanup_job_map_on_dtor_helper const&) = delete;
        cleanup_job_map_on_dtor_helper& operator= (cleanup_job_map_on_dtor_helper&&) = delete;
        Agent::job_map_t& _;
      } _cleanup_job_map_on_dtor_helper;

      WorkerManager _worker_manager;
      std::unique_ptr<Scheduler> _scheduler;

      std::mutex _cancel_mutex;
      std::mutex _scheduling_requested_guard;
      std::condition_variable _scheduling_requested_condition;
      bool _scheduling_interrupted = false;
      bool _scheduling_requested {false};
      void request_scheduling();

      std::mt19937 _random_extraction_engine;

      std::mutex mtx_subscriber_;

      gspc::logging::stream_emitter _log_emitter;
      void emit_gantt ( job_id_t const&
                      , gspc::we::type::Activity const&
                      , NotificationEvent::state_t
                      );

      gspc::util::interruptible_threadsafe_queue
        < std::pair< gspc::com::p2p::address_t
                   , std::shared_ptr<events::SchedulerEvent>
                   >
        > _event_queue;

      gspc::scheduler::com::NetworkStrategy _network_strategy;

      gspc::we::layer _workflow_engine;

      ::boost::strict_scoped_thread<> _scheduling_thread;
      gspc::util::finally_t<std::function<void()>> _interrupt_scheduling_thread;
      void scheduling_thread();

      //! \note In order to call the correct abstract functions, the
      //! event handling thread needs to be in the class that defines
      //! the handlers, while the handling is done the same way for
      //! all classes. (see issue #618)
      void handle_events();

      std::unique_ptr<gspc::iml::Client> _virtual_memory_api;

      ::boost::strict_scoped_thread<> _event_handler_thread;
      decltype (_event_queue)::interrupt_on_scope_exit _interrupt_event_queue;

      struct child_proxy
      {
        child_proxy (Agent*, gspc::com::p2p::address_t const&);

        void worker_registration_response
          (std::optional<std::exception_ptr>) const;

        void submit_job
          ( std::optional<job_id_t>
          , gspc::we::type::Activity
          , std::optional<std::string> const&
          , std::set<worker_id_t> const&
          ) const;
        void cancel_job (job_id_t) const;

        void job_failed_ack (job_id_t) const;
        void job_finished_ack (job_id_t) const;

        void put_token ( job_id_t
                       , std::string put_token_id
                       , std::string place_name
                       , gspc::pnet::type::value::value_type
                       ) const;

        void workflow_response ( job_id_t
                               , std::string workflow_response_id
                               , std::string place_name
                               , gspc::pnet::type::value::value_type
                               ) const;

      private:
        Agent* _that;
        gspc::com::p2p::address_t _address;
        std::string _callback_identifier;
      };

      struct parent_proxy
      {
        parent_proxy (Agent*, gspc::com::p2p::address_t const&);

        void cancel_job_ack (job_id_t) const;
        //! \todo Client only. Move to client_proxy?
        void delete_job_ack (job_id_t) const;
        void submit_job_ack (job_id_t) const;

        void put_token_response ( std::string put_token_id
                                , std::optional<std::exception_ptr>
                                ) const;
        void workflow_response_response
          ( std::string workflow_response_id
          , std::variant<std::exception_ptr, gspc::pnet::type::value::value_type>
          ) const;

      private:
        Agent* _that;
        gspc::com::p2p::address_t _address;
        std::string _callback_identifier;
      };
    };
  }
