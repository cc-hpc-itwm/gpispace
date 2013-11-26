#include "Job.hpp"

#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/scheduler/SchedulerBase.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>

using namespace std;
using namespace sdpa::events;

namespace sdpa {
  namespace daemon {
    void JobFSM_::action_reschedule_job(const MSMRescheduleEvent& evt)
    {
      DLOG(TRACE, "Reschedule the job "<<evt.jobId());
      evt.ptrScheduler()->schedule(evt.jobId());
    }

    void JobFSM_::action_job_stalled(const MSMStalledEvent& evt)
    {
      LOG(INFO, "The job "<<evt.jobId()<<" changed its status from RUNNING to STALLED");
      if(evt.ptrAgent()) {
        // notify the the job owner that the job has subtasks that are stalling
        sdpa::events::JobStalledEvent::Ptr pEvt(new sdpa::events::JobStalledEvent(evt.ptrAgent()->name(),
                                                                                  evt.jobOwner(),
                                                                                  evt.jobId()));
        evt.ptrAgent()->sendEventToMaster(pEvt);
      }
    }

    void JobFSM_::action_resume_job(const MSMResumeJobEvent& evt)
    {
      LOG(INFO, "The job "<<evt.jobId()<<" changed its status from STALLED to RUNNING");
      if(evt.ptrAgent()) {
        // notify the the job owner that the job makes progress
        sdpa::events::JobRunningEvent::Ptr pEvt(new sdpa::events::JobRunningEvent( evt.ptrAgent()->name()
                                                                                 , evt.jobOwner()
                                                                                 , evt.jobId()
                                                                                 ));
        evt.ptrAgent()->sendEventToMaster(pEvt);
      }
    }

    void JobFSM_::action_delete_job(const MSMDeleteJobEvent& e)
    {
      sdpa::events::DeleteJobAckEvent::Ptr pDelJobReply(new sdpa::events::DeleteJobAckEvent( e.from()
                                                                                             , e.to()
                                                                                             , e.jobId()) );
      e.ptrAgent()->sendEventToMaster(pDelJobReply);
    }

    void JobFSM_::action_retrieve_job_results(const MSMRetrieveJobResultsEvent& e)
    {
      const sdpa::events::JobResultsReplyEvent::Ptr pResReply(
         new sdpa::events::JobResultsReplyEvent( e.from()
                                                 , e.to()
                                                 , e.jobId()
                                                 , e.result() ));
      e.ptrAgent()->sendEventToMaster(pResReply);
    }

    Job::Job( const sdpa::job_id_t id,
              const sdpa::job_desc_t desc,
              const sdpa::job_id_t &parent
            , bool is_master_job
            )
        : id_(id)
        , desc_(desc)
        , parent_(parent)
        , _is_master_job (is_master_job)
        , result_()
        , m_error_code(0)
        , m_error_message()
    {
      start();
    }

    const sdpa::job_id_t & Job::id() const
    {
      return id_;
    }

    const sdpa::job_id_t & Job::parent() const
    {
      return parent_;
    }

    const sdpa::job_desc_t & Job::description() const
    {
      return desc_;
    }

    const sdpa::job_result_t& Job::result() const
    {
      return result_;
    }

    int Job::error_code() const
    {
      lock_type lock(mtx_);
      return m_error_code;
    }

    std::string Job::error_message () const
    {
      lock_type lock(mtx_);
      return m_error_message;
    }

    void Job::set_owner(const sdpa::worker_id_t& owner)
    {
      m_owner = owner;
    }

    sdpa::worker_id_t Job::owner() const
    {
      return m_owner;
    }

    sdpa::status::code Job::getStatus() const
    {
      lock_type lock(mtx_);
      return state_code (*current_state());
    }

    bool Job::completed() const
    {
      return sdpa::status::is_terminal (getStatus());
    }

    bool Job::is_running() const
    {
      return sdpa::status::is_running (getStatus());
    }

    bool Job::isMasterJob() const
    {
      return _is_master_job;
    }

    void Job::action_job_finished(const sdpa::events::JobFinishedEvent& evt/* evt */)
    {
      lock_type lock(mtx_);
      result_ = evt.result();
    }

    void Job::action_job_failed(const sdpa::events::JobFailedEvent& evt )
    {
      lock_type lock(mtx_);
      result_ = evt.result();
      m_error_code = evt.error_code();
      m_error_message = evt.error_message();
    }

    //transitions
    void Job::CancelJob(const sdpa::events::CancelJobEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::JobFailed(const sdpa::events::JobFailedEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::JobFinished(const sdpa::events::JobFinishedEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent* pAgent)
    {
      lock_type lock(mtx_);
      process_event(MSMDeleteJobEvent(pEvt, pAgent));
    }

    void Job::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* pAgent)
    {
      lock_type lock(mtx_);
      process_event(MSMRetrieveJobResultsEvent(pEvt, pAgent, result()));
    }

    void Job::Reschedule(sdpa::daemon::SchedulerBase*  pSched)
    {
      lock_type lock(mtx_);
      process_event(MSMRescheduleEvent (pSched, id()));
    }

    void Job::Pause(sdpa::daemon::IAgent* pAgent)
    {
      lock_type lock(mtx_);
      process_event (MSMStalledEvent (pAgent, id(), owner()));
    }

    void Job::Resume (sdpa::daemon::IAgent* pAgent)
    {
      lock_type lock(mtx_);
      process_event (MSMResumeJobEvent (pAgent, id(), owner()));
    }

    void Job::Dispatch()
    {
      lock_type lock(mtx_);
      process_event (MSMResumeJobEvent (NULL, id(), owner()));
    }
}}
