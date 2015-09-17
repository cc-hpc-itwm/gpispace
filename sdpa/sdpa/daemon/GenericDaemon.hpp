/*
 * =====================================================================================
 *
 *       Filename:  GenericDaemon.hpp
 *
 *    Description:  Generic daemon header file
 *
 *        Version:  1.0
 *        Created:  2009
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#pragma once

#include <sdpa/capability.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/com/NetworkStrategy.hpp>

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/worker_registration_response.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>

#include <sdpa/types.hpp>

#include <we/layer.hpp>
#include <we/type/schedule_data.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/shared_ptr.hpp>

#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_requirements.hpp>
#include <sdpa/types.hpp>
#include <sdpa/capability.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <network/connectable_to_address_string.hpp>
#include <fhg/util/std/pair.hpp>
#include <fhg/util/thread/set.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhglog/LogMacros.hpp>

#include <memory>
#include <mutex>
#include <random>

#define OVERWRITTEN_IN_TEST virtual

namespace sdpa {
  namespace daemon {
    class GenericDaemon : public sdpa::events::EventHandler,
                          boost::noncopyable
    {
    protected:
      struct master_network_info
      {
        master_network_info (std::string const& host, std::string const& port);
        fhg::com::host_t host;
        fhg::com::port_t port;
        boost::optional<fhg::com::p2p::address_t> address;
      };
      using master_info_t = std::map<std::string, master_network_info>;

    public:
      GenericDaemon( const std::string name
                   , const std::string url
                   , std::unique_ptr<boost::asio::io_service> peer_io_service
                   , boost::optional<boost::filesystem::path> const& vmem_socket
                   , std::vector<name_host_port_tuple> const& masters
                   , fhg::log::Logger& logger
                   , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info = boost::none
                   , bool create_wfe = false
                   );
      virtual ~GenericDaemon() = default;

      const std::string& name() const;
      boost::asio::ip::tcp::endpoint peer_local_endpoint() const;

      bool isTop() { return _master_info.empty(); }

      // WE interface
      OVERWRITTEN_IN_TEST void submit( const we::layer::id_type & id, const we::type::activity_t&);
      void cancel(const we::layer::id_type & id);
      void finished(const we::layer::id_type & id, const we::type::activity_t& result);
      void failed( const we::layer::id_type& wfId, std::string const& reason);
      void canceled(const we::layer::id_type& id);
      void discover (we::layer::id_type discover_id, we::layer::id_type job_id);
      void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t);
      void token_put
        (std::string put_token_id, boost::optional<std::exception_ptr>);

      void addCapability(const capability_t& cpb);

    protected:
      // masters and subscribers
      void unsubscribe(const fhg::com::p2p::address_t&);
      virtual void handleSubscribeEvent (fhg::com::p2p::address_t const& source, const sdpa::events::SubscribeEvent*) override;
      bool isSubscriber(const fhg::com::p2p::address_t&);
      std::list<fhg::com::p2p::address_t> subscribers (job_id_t) const;
      template<typename Event, typename... Args>
        void notify_subscribers (job_id_t job_id, Args&&... args)
      {
        for (fhg::com::p2p::address_t const& subscriber : subscribers (job_id))
        {
          sendEventToOther<Event> (subscriber, std::forward<Args> (args)...);
        }
      }
      bool subscribedFor(const fhg::com::p2p::address_t&, const sdpa::job_id_t&);

      // agent info and properties

      bool isOwnCapability(const sdpa::capability_t& cpb)
      {
    	  return (cpb.owner()==name());
      }

      // event handlers
    public:
      virtual void handleCancelJobAckEvent(fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobAckEvent* ) override = 0;
      virtual void handleCancelJobEvent(fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent*) override = 0;
      virtual void handleCapabilitiesGainedEvent(fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesGainedEvent*) override;
      virtual void handleCapabilitiesLostEvent(fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesLostEvent*) override;
      virtual void handleDeleteJobEvent(fhg::com::p2p::address_t const& source, const sdpa::events::DeleteJobEvent* ) override =0;
      virtual void handleErrorEvent(fhg::com::p2p::address_t const& source, const sdpa::events::ErrorEvent* ) override;
      virtual void handleJobFailedAckEvent(fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent* ) override;
      virtual void handleJobFailedEvent(fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedEvent* ) override = 0;
      virtual void handleJobFinishedAckEvent(fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent* ) override;
      virtual void handleJobFinishedEvent(fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedEvent* ) override = 0;
      //virtual void handleJobResultsReplyEvent (fhg::com::p2p::address_t const& source, const sdpa::events::JobResultsReplyEvent *) ?!
      virtual void handleSubmitJobAckEvent(fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobAckEvent* ) override;
      virtual void handleSubmitJobEvent(fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent* ) override;
      //virtual void handleSubscribeAckEvent (fhg::com::p2p::address_t const& source, const sdpa::events::SubscribeAckEvent*) ?!
      virtual void handle_worker_registration_response(fhg::com::p2p::address_t const& source, const sdpa::events::worker_registration_response*) override;
      virtual void handleWorkerRegistrationEvent(fhg::com::p2p::address_t const& source, const sdpa::events::WorkerRegistrationEvent* ) override;
      virtual void handleQueryJobStatusEvent(fhg::com::p2p::address_t const& source, const sdpa::events::QueryJobStatusEvent* ) override;
      virtual void handleRetrieveJobResultsEvent(fhg::com::p2p::address_t const& source, const sdpa::events::RetrieveJobResultsEvent* ) override;
      virtual void handleBacklogNoLongerFullEvent (fhg::com::p2p::address_t const& source, const events::BacklogNoLongerFullEvent*) override;

      virtual void handleDiscoverJobStatesReplyEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesReplyEvent*) override;
      virtual void handleDiscoverJobStatesEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent*) override;

      virtual void handle_put_token (fhg::com::p2p::address_t const& source, const events::put_token*) override;
      virtual void handle_put_token_response (fhg::com::p2p::address_t const& source, const events::put_token_response*) override;

    private:
      // event communication
      template<typename Event, typename... Args>
        void sendEventToOther
        (fhg::com::p2p::address_t const& address, Args... args)
      {
        _network_strategy.perform<Event> (address, std::forward<Args> (args)...);
      }
      void delay (std::function<void()>);

    public:
      // registration
      void requestRegistration (master_info_t::iterator const&);
      void request_registration_soon (master_info_t::iterator const&);

      // workflow engine
    public:
      const std::unique_ptr<we::layer>& workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return !!ptr_workflow_engine_;}

      // workers
      void serveJob(std::set<worker_id_t> const&, const job_id_t&);

    protected:
      // jobs
      std::string gen_id();

    private:
      Job* addJob ( const sdpa::job_id_t& job_id
                  , const job_desc_t desc
                  , boost::optional<master_info_t::iterator> owner
                  , const job_requirements_t& job_req_list
                  );

    public:
      Job* findJob(const sdpa::job_id_t& job_id ) const;
      void deleteJob(const sdpa::job_id_t& job_id);

    private:
      void delayed_cancel (const we::layer::id_type&);
      void delayed_discover (we::layer::id_type discover_id, we::layer::id_type);

      // data members
    protected:
      fhg::log::Logger& _logger;

      std::string _name;

      friend struct sdpa::opaque_job_master_t::implementation;

      master_info_t _master_info;

      boost::optional<master_info_t::iterator> master_by_address
        (fhg::com::p2p::address_t const&);

      typedef std::unordered_map<fhg::com::p2p::address_t, job_id_list_t> subscriber_map_t;
      subscriber_map_t _subscriptions;

    private:
      std::unordered_map<std::pair<job_id_t, job_id_t>, fhg::com::p2p::address_t>
        _discover_sources;

      std::unordered_map<std::string, fhg::com::p2p::address_t> _put_token_source;

    private:
      typedef std::unordered_map<sdpa::job_id_t, sdpa::daemon::Job*>
        job_map_t;

      mutable boost::mutex _job_map_mutex;
      job_map_t job_map_;
      struct cleanup_job_map_on_dtor_helper
      {
        cleanup_job_map_on_dtor_helper (GenericDaemon::job_map_t&);
        ~cleanup_job_map_on_dtor_helper();
        GenericDaemon::job_map_t& _;
      } _cleanup_job_map_on_dtor_helper;

    protected:
      WorkerManager _worker_manager;
      CoallocationScheduler _scheduler;

      boost::mutex _scheduling_thread_mutex;
      boost::condition_variable _scheduling_thread_notifier;
      void request_scheduling();

      boost::optional<std::mt19937> _random_extraction_engine;

    private:

      std::mutex mtx_subscriber_;
      std::mutex mtx_cpb_;

      sdpa::capabilities_set_t m_capabilities;

    protected:
      std::unique_ptr<NotificationService> m_guiService;

    private:
      boost::posix_time::time_duration _registration_timeout;

      void do_registration_after_sleep (master_info_t::iterator const&);

      fhg::thread::queue< std::pair< fhg::com::p2p::address_t
                                   , boost::shared_ptr<events::SDPAEvent>
                                   >
                        > _event_queue;

      sdpa::com::NetworkStrategy _network_strategy;

      std::unique_ptr<we::layer> ptr_workflow_engine_;

      fhg::thread::set _registration_threads;

      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _scheduling_thread;
      void scheduling_thread();

      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _event_handler_thread;
      void handle_events();

      struct virtual_memory_api
      {
        virtual_memory_api
          (fhg::log::Logger&, boost::filesystem::path const& socket);
        std::function<double (std::string const&)>
          transfer_costs (const we::type::activity_t& activity);
      private:
        gpi::pc::client::api_t _;
      };
      std::unique_ptr<virtual_memory_api> _virtual_memory_api;
    protected:
      struct child_proxy
      {
        child_proxy (GenericDaemon*, fhg::com::p2p::address_t const&);

        void worker_registration_response
          (boost::optional<std::exception_ptr>) const;

        void submit_job
          (boost::optional<job_id_t>, job_desc_t, std::set<worker_id_t> const&) const;
        void cancel_job (job_id_t) const;

        void job_failed_ack (job_id_t) const;
        void job_finished_ack (job_id_t) const;

        void discover_job_states (job_id_t, job_id_t discover_id) const;

        void put_token ( job_id_t
                       , std::string put_token_id
                       , std::string place_name
                       , pnet::type::value::value_type
                       ) const;

      private:
        GenericDaemon* _that;
        fhg::com::p2p::address_t _address;
        std::string _callback_identifier;
      };

      struct parent_proxy
      {
        parent_proxy (GenericDaemon*, fhg::com::p2p::address_t const&);
        parent_proxy (GenericDaemon*, master_info_t::iterator const&);
        parent_proxy (GenericDaemon*, opaque_job_master_t const&);

        void worker_registration (capabilities_set_t) const;
        void notify_shutdown() const;

        void job_failed (job_id_t, std::string error_message) const;
        void job_finished (job_id_t, job_result_t) const;

        void cancel_job_ack (job_id_t) const;
        //! \todo Client only. Move to client_proxy?
        void delete_job_ack (job_id_t) const;
        void submit_job_ack (job_id_t) const;

        void capabilities_gained (capabilities_set_t) const;
        void capabilities_lost (capabilities_set_t) const;

        void discover_job_states_reply
          (job_id_t discover_id, discovery_info_t) const;
        //! \todo Client only. Move to client_proxy?
        void query_job_status_reply
          (job_id_t, status::code, std::string error_message) const;
        //! \todo Client only. Move to client_proxy?
        void retrieve_job_results_reply (job_id_t, job_result_t) const;

        void put_token_response ( std::string put_token_id
                                , boost::optional<std::exception_ptr>
                                ) const;

      private:
        GenericDaemon* _that;
        fhg::com::p2p::address_t _address;
        std::string _callback_identifier;
      };
    };
  }
}
