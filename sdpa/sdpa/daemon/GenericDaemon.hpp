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

#include <fhg/assert.hpp>

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

#include <we/type/schedule_data.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/types.hpp>
#include <sdpa/capability.hpp>

#include <fhg/util/thread/set.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhglog/fhglog.hpp>

namespace sdpa {
  struct job_info_t{
    job_info_t( const sdpa::agent_id_t& disc_issuer,
                const sdpa::job_id_t& job_id,
                const sdpa::status::code& job_status )
      : _disc_issuer(disc_issuer)
        , _job_id(job_id)
        , _job_status(job_status)
    {}

    sdpa::agent_id_t disc_issuer() const { return _disc_issuer; }
    sdpa::job_id_t job_id() const { return _job_id; }
    sdpa::status::code job_status() const { return _job_status; }

  private:
    sdpa::agent_id_t _disc_issuer;
    sdpa::job_id_t _job_id;
    sdpa::status::code _job_status;
  };
}

typedef boost::unordered_map<we::layer::id_type, sdpa::job_info_t> map_discover_ids_t;

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
      virtual ~GenericDaemon();

      const std::string& name() const;

      void removeMasters(const agent_id_list_t& );
      size_t numberOfMasterAgents() { return m_arrMasterInfo.size(); }

      bool isTop() { return m_arrMasterInfo.empty(); }

      // WE interface
      OVERWRITTEN_IN_TEST void submit( const we::layer::id_type & id, const we::type::activity_t&);
      void cancel(const we::layer::id_type & id);
      void finished(const we::layer::id_type & id, const we::type::activity_t& result);
      void failed( const we::layer::id_type& wfId, std::string const& reason);
      void canceled(const we::layer::id_type& id);
      OVERWRITTEN_IN_TEST void discover (we::layer::id_type discover_id, we::layer::id_type job_id);
      OVERWRITTEN_IN_TEST void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t);

      void addCapability(const capability_t& cpb);

      boost::shared_ptr<CoallocationScheduler> scheduler() const {return ptr_scheduler_;}
    protected:
      // masters and subscribers
      void unsubscribe(const sdpa::agent_id_t&);
      void subscribe(const sdpa::agent_id_t&, const sdpa::job_id_list_t&);
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
      virtual void handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt );
      virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
      virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );
      virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
      virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );

      virtual void handleDiscoverJobStatesReplyEvent
        (const sdpa::events::DiscoverJobStatesReplyEvent*);
      virtual void handleDiscoverJobStatesEvent
        (const sdpa::events::DiscoverJobStatesEvent*);

      // event communication
      void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
      OVERWRITTEN_IN_TEST void sendEventToOther(const sdpa::events::SDPAEvent::Ptr& e);
    private:
      void delay (boost::function<void()>);

    public:
      // registration
      void requestRegistration(const MasterInfo& masterInfo);
      void request_registration_soon (const MasterInfo& info);

      // workflow engine
    public:
      we::layer* workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return ptr_workflow_engine_;}

      // workers
      OVERWRITTEN_IN_TEST void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId);

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
        boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

        Job* pJob = new Job( job_id, desc, is_master_job, owner );

        if (!job_map_.insert(std::make_pair (job_id, pJob)).second)
        {
          delete pJob;
          throw std::runtime_error ("job with same id already exists");
        }

        if (!job_req_list.empty())
          job_requirements_.insert(std::make_pair(job_id, job_req_list));

        return pJob;
      }

    public:
      void TEST_add_dummy_job
        (const sdpa::job_id_t& job_id, const job_requirements_t& req_list)
      {
        addJob (job_id, job_id, false, "", req_list);
      }
      Job* findJob(const sdpa::job_id_t& job_id ) const
      {
        boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

        const job_map_t::const_iterator it (job_map_.find( job_id ));
        return it != job_map_.end() ? it->second : NULL;
      }
      void deleteJob(const sdpa::job_id_t& job_id)
      {
        boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

        job_requirements_.erase (job_id);

        const job_map_t::const_iterator it (job_map_.find( job_id ));
        if (it != job_map_.end())
        {
          delete it->second;
          job_map_.erase (it);
        }
        else
        {
          throw JobNotFoundException();
        }
      }
      //! \todo Why doesn't every job have an entry here?
      const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const
      {
        boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

        const requirements_map_t::const_iterator it (job_requirements_.find (jobId));
        return it != job_requirements_.end() ? it->second : job_requirements_t();
      }

      // forwaring to scheduler() only:
      Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id ) const
      {
        return scheduler()->findWorker(worker_id);
      }

    private:
      void delayed_cancel (const we::layer::id_type&);
      void delayed_discover (we::layer::id_type discover_id, we::layer::id_type);

      // data members
    protected:
      fhg::log::Logger::ptr_t _logger;

      std::string _name;

      mutex_type mtx_;

      sdpa::master_info_list_t m_arrMasterInfo;
      sdpa::subscriber_map_t m_listSubscribers;
      map_discover_ids_t m_map_discover_ids;

    private:
      typedef boost::unordered_map<sdpa::job_id_t, job_requirements_t>
        requirements_map_t;
      typedef boost::unordered_map<sdpa::job_id_t, sdpa::daemon::Job*>
        job_map_t;

      mutable boost::mutex _job_map_and_requirements_mutex;
      job_map_t job_map_;
      requirements_map_t job_requirements_;

    protected:
      boost::shared_ptr<CoallocationScheduler> ptr_scheduler_;

      boost::mutex _scheduling_thread_mutex;
      boost::condition_variable _scheduling_thread_notifier;
      boost::thread _scheduling_thread;
      void scheduling_thread();
      void request_scheduling();

      boost::optional<boost::mt19937> _random_extraction_engine;
      we::layer* ptr_workflow_engine_;

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
      fhg::thread::set _registration_threads;

      void do_registration_after_sleep (const MasterInfo);


      fhg::thread::queue<boost::shared_ptr<events::SDPAEvent> > _event_queue;
      boost::thread _event_handler_thread;
      void handle_events();

      fhg::com::kvs::kvsc_ptr_t _kvs_client;
      boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;

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
