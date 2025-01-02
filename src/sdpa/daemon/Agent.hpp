// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <drts/scheduler_types.hpp>
#include <sdpa/capability.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/worker_registration_response.hpp>
#include <sdpa/requirements_and_preferences.hpp>
#include <sdpa/types.hpp>

#include <gspc/iml/Client.hpp>

#include <logging/stream_emitter.hpp>

#include <we/layer.hpp>
#include <we/type/Activity.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hash/std/pair.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
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

namespace sdpa {
  namespace daemon {
    class Agent final : public sdpa::events::EventHandler
    {
    public:
      Agent ( std::string name
            , std::string url
            , std::unique_ptr<::boost::asio::io_service> peer_io_service
            , std::optional<::boost::filesystem::path> const& vmem_socket
            , gspc::Certificates const& certificates
            );
      ~Agent() override = default;
      Agent (Agent const&) = delete;
      Agent (Agent&&) = delete;
      Agent& operator= (Agent const&) = delete;
      Agent& operator= (Agent&&) = delete;

      std::string const& name() const;
      ::boost::asio::ip::tcp::endpoint peer_local_endpoint() const;
      fhg::logging::endpoint logger_registration_endpoint() const;
      fhg::logging::stream_emitter& log_emitter();

    public:
      // WE interface
      void submit( we::layer::id_type const& id, we::type::Activity);
      void cancel (we::layer::id_type const& id);
      void finished (we::layer::id_type const& id, we::type::Activity const& result);
      void failed( we::layer::id_type const& wfId, std::string const& reason);
      void canceled (we::layer::id_type const& id);
      void token_put
        (std::string put_token_id, ::boost::optional<std::exception_ptr>);
      void workflow_response_response
        ( std::string workflow_response_id
        , std::variant<std::exception_ptr, pnet::type::value::value_type>
        );

      void addCapability (Capability const& cpb);
      bool workflow_submission_is_allowed (Job const* pJob);

    private:
      // parents and subscribers
      void unsubscribe (fhg::com::p2p::address_t const&);
      void handleSubscribeEvent (fhg::com::p2p::address_t const& source, const sdpa::events::SubscribeEvent*) override;

      bool isSubscriber (fhg::com::p2p::address_t const&, job_id_t const&);

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
        ( fhg::com::p2p::address_t const&
        , sdpa::events::CancelJobEvent const*
        ) override;
      void handleDeleteJobEvent
        ( fhg::com::p2p::address_t const&
        , events::DeleteJobEvent const*
        ) override;
      void handleErrorEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::ErrorEvent*
        ) override;
      void handleJobFailedAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::JobFailedAckEvent*
        ) override;
      void handleJobFinishedAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::JobFinishedAckEvent*
        ) override;
      void handleSubmitJobAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::SubmitJobAckEvent*
        ) override;
      void handleSubmitJobEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::SubmitJobEvent*
        ) override;
      void handle_worker_registration_response
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::worker_registration_response*
        ) override;
      void handleWorkerRegistrationEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::WorkerRegistrationEvent*
        ) override;
      void handle_put_token
        ( fhg::com::p2p::address_t const&
        , const events::put_token*
        ) override;
      void handle_put_token_response
        ( fhg::com::p2p::address_t const&
        , const events::put_token_response*
        ) override;
      void handle_workflow_response
        ( fhg::com::p2p::address_t const&
        , const events::workflow_response*
        ) override;
      void handle_workflow_response_response
        ( fhg::com::p2p::address_t const&
        , events::workflow_response_response const*
        ) override;
      void handleJobFailedEvent
        ( fhg::com::p2p::address_t const&
        , events::JobFailedEvent const*
        ) override;
      void handleJobFinishedEvent
        ( fhg::com::p2p::address_t const&
        , events::JobFinishedEvent const*
        ) override;
      void handleCancelJobAckEvent
        ( fhg::com::p2p::address_t const&
        , events::CancelJobAckEvent const*
        ) override;

      // event communication
      template<typename Event, typename... Args>
        void sendEventToOther
        (fhg::com::p2p::address_t const& address, Args... args)
      {
        _network_strategy.perform<Event> (address, std::forward<Args> (args)...);
      }
      void delay (std::function<void()>);

      // workflow engine
      bool workflow_engine_submit (job_id_t, Job*);

      void handle_job_termination
        (fhg::com::p2p::address_t const&, Job*, terminal_state const&);

      void workflow_finished
        (we::layer::id_type const&, we::type::Activity const&);
      void workflow_failed
        (we::layer::id_type const&, std::string const&);
      void workflow_canceled (we::layer::id_type const&);
      void token_put_in_workflow
        (std::string, ::boost::optional<std::exception_ptr>);
      void workflow_engine_workflow_response_response
        ( std::string
        , std::variant<std::exception_ptr, pnet::type::value::value_type>
        );

      //! \todo aggregated results for coallocation jobs and sub jobs
      void job_finished (Job*, we::type::Activity const&);
      void job_failed (Job*, std::string const& reason);
      void job_canceled (Job*);

      // workers
      void serveJob
        ( WorkerSet const&
        , Implementation const&
        , job_id_t const&
        , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
        );

      // jobs
      std::string gen_id();

      Job* addJob ( sdpa::job_id_t const& job_id
                  , we::type::Activity
                  , job_source
                  , job_handler
                  );
      Job* addJob ( sdpa::job_id_t const& job_id
                  , we::type::Activity
                  , job_source
                  , job_handler
                  , Requirements_and_preferences
                  );
      Job* addJobWithNoPreferences ( sdpa::job_id_t const&
                                   , we::type::Activity
                                   , job_source
                                   , job_handler
                                   );

      Job* findJob (sdpa::job_id_t const& job_id ) const;
      Job* require_job (job_id_t const&, std::string const& error) const;
      void deleteJob (sdpa::job_id_t const& job_id);

      void cancel_worker_handled_job (we::layer::id_type const&);

      std::string _name;

      using subscriber_relation_type =
        ::boost::bimap
        < ::boost::bimaps::unordered_multiset_of<fhg::com::p2p::address_t>
        , ::boost::bimaps::unordered_multiset_of<job_id_t>
        , ::boost::bimaps::set_of_relation<>
        >;

      subscriber_relation_type _subscriptions;

      std::unordered_map<std::string, fhg::com::p2p::address_t> _put_token_source;
      std::unordered_map<std::string, fhg::com::p2p::address_t>
        _workflow_response_source;

      using job_map_t = std::unordered_map<sdpa::job_id_t, sdpa::daemon::Job*>;

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

      fhg::logging::stream_emitter _log_emitter;
      void emit_gantt ( job_id_t const&
                      , we::type::Activity const&
                      , NotificationEvent::state_t
                      );

      fhg::util::interruptible_threadsafe_queue
        < std::pair< fhg::com::p2p::address_t
                   , ::boost::shared_ptr<events::SDPAEvent>
                   >
        > _event_queue;

      sdpa::com::NetworkStrategy _network_strategy;

      we::layer _workflow_engine;

      ::boost::strict_scoped_thread<> _scheduling_thread;
      fhg::util::finally_t<std::function<void()>> _interrupt_scheduling_thread;
      void scheduling_thread();

      //! \note In order to call the correct abstract functions, the
      //! event handling thread needs to be in the class that defines
      //! the handlers, while the handling is done the same way for
      //! all classes. (see issue #618)
      void handle_events();

      std::unique_ptr<iml::Client> _virtual_memory_api;

      ::boost::strict_scoped_thread<> _event_handler_thread;
      decltype (_event_queue)::interrupt_on_scope_exit _interrupt_event_queue;

      struct child_proxy
      {
        child_proxy (Agent*, fhg::com::p2p::address_t const&);

        void worker_registration_response
          (::boost::optional<std::exception_ptr>) const;

        void submit_job
          ( ::boost::optional<job_id_t>
          , we::type::Activity
          , ::boost::optional<std::string> const&
          , std::set<worker_id_t> const&
          ) const;
        void cancel_job (job_id_t) const;

        void job_failed_ack (job_id_t) const;
        void job_finished_ack (job_id_t) const;

        void put_token ( job_id_t
                       , std::string put_token_id
                       , std::string place_name
                       , pnet::type::value::value_type
                       ) const;

        void workflow_response ( job_id_t
                               , std::string workflow_response_id
                               , std::string place_name
                               , pnet::type::value::value_type
                               ) const;

      private:
        Agent* _that;
        fhg::com::p2p::address_t _address;
        std::string _callback_identifier;
      };

      struct parent_proxy
      {
        parent_proxy (Agent*, fhg::com::p2p::address_t const&);

        void cancel_job_ack (job_id_t) const;
        //! \todo Client only. Move to client_proxy?
        void delete_job_ack (job_id_t) const;
        void submit_job_ack (job_id_t) const;

        void put_token_response ( std::string put_token_id
                                , ::boost::optional<std::exception_ptr>
                                ) const;
        void workflow_response_response
          ( std::string workflow_response_id
          , std::variant<std::exception_ptr, pnet::type::value::value_type>
          ) const;

      private:
        Agent* _that;
        fhg::com::p2p::address_t _address;
        std::string _callback_identifier;
      };
    };
  }
}
