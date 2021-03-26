// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#pragma once

#include <sdpa/capability.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
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
#include <sdpa/master_network_info.hpp>
#include <sdpa/requirements_and_preferences.hpp>
#include <sdpa/types.hpp>

#include <iml/Client.hpp>

#include <logging/stream_emitter.hpp>

#include <we/layer.hpp>
#include <we/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/util/thread/set.hpp>
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

#include <chrono>
#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <queue>
#include <random>

namespace sdpa {
  namespace daemon {
    class Agent final : public sdpa::events::EventHandler
                              , boost::noncopyable
    {
    public:
      Agent( const std::string name
                   , const std::string url
                   , std::unique_ptr<boost::asio::io_service> peer_io_service
                   , boost::optional<boost::filesystem::path> const& vmem_socket
                   , master_info_t masters
                   , bool create_wfe
                   , fhg::com::Certificates const& certificates
                   );
      virtual ~Agent() override = default;

      const std::string& name() const;
      boost::asio::ip::tcp::endpoint peer_local_endpoint() const;
      fhg::logging::endpoint logger_registration_endpoint() const;
      fhg::logging::stream_emitter& log_emitter();

    public:
      // WE interface
      void submit( const we::layer::id_type & id, we::type::activity_t);
      void cancel(const we::layer::id_type & id);
      void finished(const we::layer::id_type & id, const we::type::activity_t& result);
      void failed( const we::layer::id_type& wfId, std::string const& reason);
      void canceled(const we::layer::id_type& id);
      void token_put
        (std::string put_token_id, boost::optional<std::exception_ptr>);
      void workflow_response_response
        ( std::string workflow_response_id
        , boost::variant<std::exception_ptr, pnet::type::value::value_type>
        );

      void addCapability(const capability_t& cpb);

    private:
      // masters and subscribers
      void unsubscribe(const fhg::com::p2p::address_t&);
      virtual void handleSubscribeEvent (fhg::com::p2p::address_t const& source, const sdpa::events::SubscribeEvent*) override;

      bool isSubscriber (fhg::com::p2p::address_t const&, job_id_t const&);

      template<typename Event, typename... Args>
        void notify_subscribers (job_id_t job_id, Args&&... args)
      {
        std::lock_guard<std::mutex> const _ (mtx_subscriber_);

        for  ( fhg::com::p2p::address_t const& subscriber
             : _subscriptions.right.equal_range (job_id)
             | boost::adaptors::map_values
             )
        {
          sendEventToOther<Event> (subscriber, std::forward<Args> (args)...);
        }

        _subscriptions.right.erase (job_id);
      }

      // agent info and properties

      bool isOwnCapability(const sdpa::capability_t& cpb)
      {
    	  return (cpb.owner()==name());
      }

      // event handlers
    private:
      virtual void handleCancelJobEvent
        ( fhg::com::p2p::address_t const&
        , sdpa::events::CancelJobEvent const*
        ) override;
      virtual void handleCapabilitiesGainedEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::CapabilitiesGainedEvent*
        ) override;
      virtual void handleCapabilitiesLostEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::CapabilitiesLostEvent*
        ) override;
      virtual void handleDeleteJobEvent
        ( fhg::com::p2p::address_t const&
        , events::DeleteJobEvent const*
        ) override;
      virtual void handleErrorEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::ErrorEvent*
        ) override;
      virtual void handleJobFailedAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::JobFailedAckEvent*
        ) override;
      virtual void handleJobFinishedAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::JobFinishedAckEvent*
        ) override;
      virtual void handleSubmitJobAckEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::SubmitJobAckEvent*
        ) override;
      virtual void handleSubmitJobEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::SubmitJobEvent*
        ) override;
      virtual void handle_worker_registration_response
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::worker_registration_response*
        ) override;
      virtual void handleWorkerRegistrationEvent
        ( fhg::com::p2p::address_t const&
        , const sdpa::events::WorkerRegistrationEvent*
        ) override;
      virtual void handleBacklogNoLongerFullEvent
        ( fhg::com::p2p::address_t const&
        , const events::BacklogNoLongerFullEvent*
        ) override;
      virtual void handle_put_token
        ( fhg::com::p2p::address_t const&
        , const events::put_token*
        ) override;
      virtual void handle_put_token_response
        ( fhg::com::p2p::address_t const&
        , const events::put_token_response*
        ) override;
      virtual void handle_workflow_response
        ( fhg::com::p2p::address_t const&
        , const events::workflow_response*
        ) override;
      virtual void handle_workflow_response_response
        ( fhg::com::p2p::address_t const&
        , events::workflow_response_response const*
        ) override;
      virtual void handleJobFailedEvent
        ( fhg::com::p2p::address_t const&
        , events::JobFailedEvent const*
        ) override;
      virtual void handleJobFinishedEvent
        ( fhg::com::p2p::address_t const&
        , events::JobFinishedEvent const*
        ) override;
      virtual void handleCancelJobAckEvent
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

      // registration
      void requestRegistration (master_network_info&);
      void request_registration_soon (master_network_info&);

      // workflow engine
      const std::unique_ptr<we::layer>& workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return !!ptr_workflow_engine_;}

      bool workflow_engine_submit (job_id_t, Job*);

      void handle_job_termination (Job*);

      void workflow_finished
        (we::layer::id_type const&, we::type::activity_t const&);
      void workflow_failed
        (we::layer::id_type const&, std::string const&);
      void workflow_canceled (we::layer::id_type const&);
      void token_put_in_workflow
        (std::string, boost::optional<std::exception_ptr>);
      void workflow_engine_workflow_response_response
        ( std::string
        , boost::variant<std::exception_ptr, pnet::type::value::value_type>
        );

      //! \todo aggregated results for coallocation jobs and sub jobs
      void job_finished (Job*, we::type::activity_t const&);
      void job_failed (Job*, std::string const& reason);
      void job_canceled (Job*);

      // workers
      void serveJob
        (WorkerSet const&, Implementation const&, const job_id_t&);

      // jobs
      std::string gen_id();

      Job* addJob ( const sdpa::job_id_t& job_id
                  , we::type::activity_t
                  , job_source
                  , job_handler
                  );
      Job* addJob ( const sdpa::job_id_t& job_id
                  , we::type::activity_t
                  , job_source
                  , job_handler
                  , Requirements_and_preferences
                  );
      Job* addJobWithNoPreferences ( const sdpa::job_id_t&
                                   , we::type::activity_t
                                   , job_source
                                   , job_handler
                                   );

      Job* findJob(const sdpa::job_id_t& job_id ) const;
      Job* require_job (job_id_t const&, std::string const& error) const;
      void deleteJob(const sdpa::job_id_t& job_id);

      void cancel_worker_handled_job (we::layer::id_type const&);

      std::string _name;

      master_info_t _master_info;

      boost::optional<master_info_t::iterator> master_by_address
        (fhg::com::p2p::address_t const&);

      using subscriber_relation_type =
        boost::bimap
        < boost::bimaps::unordered_multiset_of<fhg::com::p2p::address_t>
        , boost::bimaps::unordered_multiset_of<job_id_t>
        , boost::bimaps::set_of_relation<>
        >;

      subscriber_relation_type _subscriptions;

      std::unordered_map<std::string, fhg::com::p2p::address_t> _put_token_source;
      std::unordered_map<std::string, fhg::com::p2p::address_t>
        _workflow_response_source;

      typedef std::unordered_map<sdpa::job_id_t, sdpa::daemon::Job*>
        job_map_t;

      mutable std::mutex _job_map_mutex;
      job_map_t job_map_;
      struct cleanup_job_map_on_dtor_helper
      {
        cleanup_job_map_on_dtor_helper (Agent::job_map_t&);
        ~cleanup_job_map_on_dtor_helper();
        Agent::job_map_t& _;
      } _cleanup_job_map_on_dtor_helper;

      WorkerManager _worker_manager;
      CoallocationScheduler _scheduler;

      std::mutex _cancel_mutex;
      std::mutex _scheduling_requested_guard;
      std::condition_variable _scheduling_requested_condition;
      bool _scheduling_interrupted = false;
      bool _scheduling_requested;
      void request_scheduling();

      boost::optional<std::mt19937> _random_extraction_engine;

      std::mutex mtx_subscriber_;
      std::mutex mtx_cpb_;

      fhg::logging::stream_emitter _log_emitter;
      void emit_gantt ( job_id_t const&
                      , we::type::activity_t const&
                      , NotificationEvent::state_t
                      );

      std::chrono::seconds _registration_timeout;

      void do_registration_after_sleep (master_network_info&);

      fhg::util::interruptible_threadsafe_queue
        < std::pair< fhg::com::p2p::address_t
                   , boost::shared_ptr<events::SDPAEvent>
                   >
        > _event_queue;

      sdpa::com::NetworkStrategy _network_strategy;

      std::unique_ptr<we::layer> ptr_workflow_engine_;

      fhg::thread::set _registration_threads;

      boost::strict_scoped_thread<> _scheduling_thread;
      fhg::util::finally_t<std::function<void()>> _interrupt_scheduling_thread;
      void scheduling_thread();

      //! \note In order to call the correct abstract functions, the
      //! event handling thread needs to be in the class that defines
      //! the handlers, while the handling is done the same way for
      //! all classes. (see issue #618)
      void handle_events();

      std::unique_ptr<iml::Client> _virtual_memory_api;

      boost::strict_scoped_thread<> _event_handler_thread;
      decltype (_event_queue)::interrupt_on_scope_exit _interrupt_event_queue;

      struct child_proxy
      {
        child_proxy (Agent*, fhg::com::p2p::address_t const&);

        void worker_registration_response
          (boost::optional<std::exception_ptr>) const;

        void submit_job
          ( boost::optional<job_id_t>
          , we::type::activity_t
          , boost::optional<std::string> const&
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
        parent_proxy (Agent*, master_network_info const&);
        parent_proxy (Agent*, boost::optional<master_info_t::iterator> const&);

        void worker_registration (capabilities_set_t) const;
        void notify_shutdown() const;

        void job_failed (job_id_t, std::string error_message) const;
        void job_finished (job_id_t, we::type::activity_t) const;

        void cancel_job_ack (job_id_t) const;
        //! \todo Client only. Move to client_proxy?
        void delete_job_ack (job_id_t) const;
        void submit_job_ack (job_id_t) const;

        void capabilities_gained (capabilities_set_t) const;
        void capabilities_lost (capabilities_set_t) const;

        void put_token_response ( std::string put_token_id
                                , boost::optional<std::exception_ptr>
                                ) const;
        void workflow_response_response
          ( std::string workflow_response_id
          , boost::variant<std::exception_ptr, pnet::type::value::value_type>
          ) const;

      private:
        Agent* _that;
        fhg::com::p2p::address_t _address;
        std::string _callback_identifier;
      };
    };
  }
}
