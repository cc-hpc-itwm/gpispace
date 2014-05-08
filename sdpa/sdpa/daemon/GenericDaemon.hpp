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
#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

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
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
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

#include <fhg/util/std/pair.hpp>
#include <fhg/util/thread/set.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhglog/LogMacros.hpp>

#include <memory>
#include <random>

#define OVERWRITTEN_IN_TEST virtual

namespace sdpa {
  namespace daemon {
    class GenericDaemon : public sdpa::events::EventHandler,
                          boost::noncopyable
    {
    public:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      GenericDaemon( const std::string name
                   , const std::string url
                   , std::string kvs_host
                   , std::string kvs_port
                   , const sdpa::master_info_list_t m_arrMasterInfo =  sdpa::master_info_list_t()
                   , const boost::optional<std::string>& guiUrl = boost::none
                   , bool create_wfe = false
                   );
      virtual ~GenericDaemon() = default;

      const std::string& name() const;

      void removeMasters(const agent_id_list_t& );

      bool isTop() { return m_arrMasterInfo.empty(); }

      // WE interface
      OVERWRITTEN_IN_TEST void submit( const we::layer::id_type & id, const we::type::activity_t&);
      void cancel(const we::layer::id_type & id);
      void finished(const we::layer::id_type & id, const we::type::activity_t& result);
      void failed( const we::layer::id_type& wfId, std::string const& reason);
      void canceled(const we::layer::id_type& id);
      void discover (we::layer::id_type discover_id, we::layer::id_type job_id);
      void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t);

      void addCapability(const capability_t& cpb);

    protected:
      const CoallocationScheduler& scheduler() const {return _scheduler;}
      CoallocationScheduler& scheduler() {return _scheduler;}

      // masters and subscribers
      void unsubscribe(const sdpa::agent_id_t&);
      virtual void handleSubscribeEvent (const sdpa::events::SubscribeEvent*);
      bool isSubscriber(const sdpa::agent_id_t&);
      bool subscribedFor(const sdpa::agent_id_t&, const sdpa::job_id_t&);

      // agent info and properties

      bool isOwnCapability(const sdpa::capability_t& cpb)
      {
    	  return (cpb.owner()==name());
      }

      // event handlers
    public:
      virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* ) = 0;
      virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*) = 0;
      virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*);
      virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*);
      virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* )=0;
      virtual void handleErrorEvent(const sdpa::events::ErrorEvent* );
      virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
      virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* ) = 0;
      virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
      virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* ) = 0;
      //virtual void handleJobResultsReplyEvent (const sdpa::events::JobResultsReplyEvent *) ?!
      virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* );
      virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* );
      //virtual void handleSubscribeAckEvent (const sdpa::events::SubscribeAckEvent*) ?!
      virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
      virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );
      virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
      virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );

      virtual void handleDiscoverJobStatesReplyEvent
        (const sdpa::events::DiscoverJobStatesReplyEvent*);
      virtual void handleDiscoverJobStatesEvent
        (const sdpa::events::DiscoverJobStatesEvent*);

    protected:
      // event communication
      void sendEventToOther(const sdpa::events::SDPAEvent::Ptr& e);
    private:
      void delay (std::function<void()>);

    public:
      // registration
      void requestRegistration(const MasterInfo& masterInfo);
      void request_registration_soon (const MasterInfo& info);

      // workflow engine
    public:
      const std::unique_ptr<we::layer>& workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return !!ptr_workflow_engine_;}

      // workers
      void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId);

    protected:
      // jobs
      std::string gen_id();

    private:
      Job* addJob ( const sdpa::job_id_t& job_id
                  , const job_desc_t desc
                  , bool is_master_job
                  , const worker_id_t& owner
                  , const job_requirements_t& job_req_list
                  )
      {
        boost::mutex::scoped_lock const _ (_job_map_mutex);

        Job* pJob = new Job( job_id, desc, is_master_job, owner, job_req_list);

        if (!job_map_.emplace (job_id, pJob).second)
        {
          delete pJob;
          throw std::runtime_error ("job with same id already exists");
        }

        return pJob;
      }

    public:
      Job* findJob(const sdpa::job_id_t& job_id ) const
      {
        boost::mutex::scoped_lock const _ (_job_map_mutex);

        const job_map_t::const_iterator it (job_map_.find( job_id ));
        return it != job_map_.end() ? it->second : nullptr;
      }
      void deleteJob(const sdpa::job_id_t& job_id)
      {
        boost::mutex::scoped_lock const _ (_job_map_mutex);

        const job_map_t::const_iterator it (job_map_.find( job_id ));
        if (it != job_map_.end())
        {
          delete it->second;
          job_map_.erase (it);
        }
        else
        {
          throw std::runtime_error ("deleteJob: job not found");
        }
      }

    private:
      void delayed_cancel (const we::layer::id_type&);
      void delayed_discover (we::layer::id_type discover_id, we::layer::id_type);

      // data members
    protected:
      fhg::log::Logger::ptr_t _logger;

      std::string _name;

      sdpa::master_info_list_t m_arrMasterInfo;
      typedef std::map<agent_id_t, job_id_list_t> subscriber_map_t;
      subscriber_map_t m_listSubscribers;

    private:
      std::unordered_map<std::pair<job_id_t, job_id_t>, std::string>
        _discover_sources;

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
      CoallocationScheduler _scheduler;

      boost::mutex _scheduling_thread_mutex;
      boost::condition_variable _scheduling_thread_notifier;
      void request_scheduling();

      boost::optional<std::mt19937> _random_extraction_engine;

    private:

      mutex_type mtx_subscriber_;
    protected:
      mutex_type mtx_master_;
    private:
      mutex_type mtx_cpb_;

      sdpa::capabilities_set_t m_capabilities;

    protected:
      boost::optional<NotificationService> m_guiService;

    private:
      unsigned int _max_consecutive_registration_attempts;
      unsigned int _max_consecutive_network_faults;
      boost::posix_time::time_duration _registration_timeout;

      void do_registration_after_sleep (const MasterInfo);

      fhg::thread::queue<boost::shared_ptr<events::SDPAEvent>> _event_queue;

      fhg::com::kvs::kvsc_ptr_t _kvs_client;
      sdpa::com::NetworkStrategy _network_strategy;

      std::unique_ptr<we::layer> ptr_workflow_engine_;

      fhg::thread::set _registration_threads;

      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _scheduling_thread;
      void scheduling_thread();

      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _event_handler_thread;
      void handle_events();

    protected:
      struct child_proxy
      {
        child_proxy (GenericDaemon*, worker_id_t);

        void worker_registration_ack() const;

        void submit_job
          (boost::optional<job_id_t>, job_desc_t, sdpa::worker_id_list_t) const;
        void cancel_job (job_id_t) const;

        void job_failed_ack (job_id_t) const;
        void job_finished_ack (job_id_t) const;

        void discover_job_states (job_id_t, job_id_t discover_id) const;

      private:
        GenericDaemon* _that;
        worker_id_t _name;
      };

      struct parent_proxy
      {
        parent_proxy (GenericDaemon*, worker_id_t);

        void worker_registration
          (boost::optional<unsigned int> capacity, capabilities_set_t) const;
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

      private:
        GenericDaemon* _that;
        worker_id_t _name;
      };
    };
  }
}

#endif
