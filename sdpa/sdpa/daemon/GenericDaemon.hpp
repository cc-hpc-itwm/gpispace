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
#include <sdpa/daemon/scheduler/SchedulerBase.hpp>
#include <sdpa/daemon/JobManager.hpp>
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
      typedef boost::condition_variable_any condition_type;
      typedef boost::shared_ptr<GenericDaemon> ptr_t;

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
      virtual void finished(const we::layer::id_type & id, const we::type::activity_t& result);
      virtual void failed( const we::layer::id_type& wfId, std::string const& reason);
      OVERWRITTEN_IN_TEST void canceled(const we::layer::id_type& id);
      OVERWRITTEN_IN_TEST void discover (we::layer::id_type discover_id, we::layer::id_type job_id);
      OVERWRITTEN_IN_TEST void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t);

      void addCapability(const capability_t& cpb);
      void getCapabilities(sdpa::capabilities_set_t& cpbset);

      SchedulerBase::ptr_t scheduler() const {return ptr_scheduler_;}

      void stop_all();

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

      virtual void handleDiscoverJobStatesReplyEvent
        (const sdpa::events::DiscoverJobStatesReplyEvent*);

      // event communication
      void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
      OVERWRITTEN_IN_TEST void sendEventToOther(const sdpa::events::SDPAEvent::Ptr& e);

      // registration
      void requestRegistration(const MasterInfo& masterInfo);
      void request_registration_soon (const MasterInfo& info);

    protected:
      void registerWorker(const sdpa::events::WorkerRegistrationEvent& evtRegWorker);

      // workflow engine
    public:
      we::layer* workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return ptr_workflow_engine_;}

    protected:
      // workflow engine notifications
      OVERWRITTEN_IN_TEST void submitWorkflow(const job_id_t& id);

    public:
      // workers
      OVERWRITTEN_IN_TEST void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId);

    protected:
      // jobs
      std::string gen_id();

    public:
      // forwarding to jobManager() only:
      void TEST_add_dummy_job
        (const sdpa::job_id_t& job_id, const job_requirements_t& req_list)
      {
        return jobManager().addJob (job_id, job_id, false, "", req_list);
      }
      bool hasJobs()
      {
        return jobManager().hasJobs();
      }
      Job* findJob(const sdpa::job_id_t& job_id ) const
      {
        return jobManager().findJob(job_id);
      }
      void deleteJob(const sdpa::job_id_t& jobId)
      {
        jobManager().deleteJob(jobId);
      }
      const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const
      {
        return jobManager().getJobRequirements(jobId);
      }

      // forwaring to scheduler() only:
      Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id ) const
      {
        return scheduler()->findWorker(worker_id);
      }

      bool hasWorker(const Worker::worker_id_t& worker_id ) const
      {
        return scheduler()->hasWorker(worker_id);
      }

    protected:
      const JobManager& jobManager() const { return _job_manager; }
      JobManager& jobManager() { return _job_manager; }

      // data members
    protected:
      fhg::log::Logger::ptr_t _logger;

      std::string _name;

      mutex_type mtx_;

      sdpa::master_info_list_t m_arrMasterInfo;
      sdpa::subscriber_map_t m_listSubscribers;
      map_discover_ids_t m_map_discover_ids;

    protected:
      JobManager _job_manager;
      SchedulerBase::ptr_t ptr_scheduler_;
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

      bool _event_handling_enabled;
      fhg::thread::queue<boost::shared_ptr<events::SDPAEvent> > _event_queue;
      boost::thread _event_handler_thread;
      void handle_events();

      fhg::com::kvs::kvsc_ptr_t _kvs_client;
      boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;
    };
  }
}

#endif
