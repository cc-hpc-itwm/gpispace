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

#include <seda/Stage.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/logging.hpp>
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
#include <we/type/user_data.hpp>

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

      GenericDaemon(const std::string name = "orchestrator_0",
                    const sdpa::master_info_list_t m_arrMasterInfo =  sdpa::master_info_list_t(),
                    unsigned int rank = 0
                   , const boost::optional<std::string>& guiUrl = boost::none
                   , bool create_wfe = false
                   );
      virtual ~GenericDaemon() {}

      SDPA_DECLARE_LOGGER();
      const std::string& name() const;

      const unsigned int& rank() const { return m_nRank; }
      unsigned int& rank() { return m_nRank; }
      virtual const std::string url() const = 0;
      const sdpa::worker_id_t& agent_uuid() { return m_strAgentUID; }

      void start_agent();

      void shutdown();

      void removeMasters(const agent_id_list_t& );
      size_t numberOfMasterAgents() { return m_arrMasterInfo.size(); }

      virtual bool isTop() { return false; }

      // WE interface
      void submit( const we::mgmt::layer::id_type & id
                         , const we::mgmt::layer::encoded_type&
                         , const requirement_list_t& = requirement_list_t()
                         , const we::type::schedule_data& = we::type::schedule_data()
                         , const we::type::user_data & = we::type::user_data ()
                         );
      bool cancel(const we::mgmt::layer::id_type & id, const we::mgmt::layer::reason_type& reason);
      virtual bool finished(const we::mgmt::layer::id_type & id, const we::mgmt::layer::result_type& result);
      virtual bool failed( const we::mgmt::layer::id_type& wfId, const we::mgmt::layer::result_type& res, int errc, std::string const& reason);
      bool canceled(const we::mgmt::layer::id_type& id);
      virtual void pause(const job_id_t& id ) = 0;
      virtual void resume(const job_id_t& id ) = 0;

      void addCapability(const capability_t& cpb);
      void getCapabilities(sdpa::capabilities_set_t& cpbset);

      SchedulerBase::ptr_t scheduler() const {return ptr_scheduler_;}
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
      virtual void perform(const boost::shared_ptr<events::SDPAEvent>&);
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
      virtual void handleJobRunningEvent (const sdpa::events::JobRunningEvent *);
      virtual void handleJobStalledEvent (const sdpa::events::JobStalledEvent *);
      virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* );
      virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* );
      //virtual void handleSubscribeAckEvent (const sdpa::events::SubscribeAckEvent*) ?!
      virtual void handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt );
      virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
      virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );

      // event communication
      OVERWRITTEN_IN_TEST void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
      OVERWRITTEN_IN_TEST void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout
      OVERWRITTEN_IN_TEST void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout

      // registration
      void requestRegistration(const MasterInfo& masterInfo);
      void request_registration_soon (const MasterInfo& info);

    protected:
      void registerWorker(const sdpa::events::WorkerRegistrationEvent& evtRegWorker);

      // workflow engine
    public:
      we::mgmt::layer* workflowEngine() const { return ptr_workflow_engine_; }
      bool hasWorkflowEngine() const { return ptr_workflow_engine_;}

    protected:
      // workflow engine notifications
      OVERWRITTEN_IN_TEST void submitWorkflow(const job_id_t& id);

    public:
      // workers
      OVERWRITTEN_IN_TEST void serveJob(const Worker::worker_id_t&, const job_id_t&);
      OVERWRITTEN_IN_TEST void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId);

    private:
      // jobs
      std::string gen_id() { return sdpa::JobId ().str (); }

    public:
      // forwarding to jobManager() only:
      //void addJob( const sdpa::job_id_t& jid, Job* pJob, const job_requirements_t& reqList = job_requirements_t())
      void addJob ( const sdpa::job_id_t& job_id
                    , const job_desc_t desc
                    , const job_id_t &parent
                    , bool is_master_job
                    , const worker_id_t& owner
                    , const job_requirements_t& req_list = job_requirements_t()
                  )
      {
        return jobManager().addJob(job_id, desc, parent, is_master_job, owner, req_list);
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
      void getWorkerCapabilities(const Worker::worker_id_t& worker_id, sdpa::capabilities_set_t& wCpbset)
      {
        scheduler()->getWorkerCapabilities(worker_id, wCpbset);
      }

      bool noChildJobStalled(const sdpa::job_id_t& jobId) const;
      bool noChildJobRunning(const sdpa::job_id_t& jobId) const;

    protected:
      const JobManager& jobManager() const { return _job_manager; }
      JobManager& jobManager() { return _job_manager; }

      // data members
    protected:
      std::string _name;

      mutex_type mtx_;

      sdpa::master_info_list_t m_arrMasterInfo;
      sdpa::subscriber_map_t m_listSubscribers;

    private:
      boost::shared_ptr<seda::Stage<events::SDPAEvent> > ptr_daemon_stage_;
      boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;
      boost::shared_ptr<seda::Stage<events::SDPAEvent> > _network_stage;

    protected:
      JobManager _job_manager;
      SchedulerBase::ptr_t ptr_scheduler_;
      we::mgmt::layer* ptr_workflow_engine_;

    private:

      unsigned int m_nRank;
      sdpa::worker_id_t m_strAgentUID;

      mutex_type mtx_subscriber_;
      mutex_type mtx_master_;
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
    };
  }
}

#endif
