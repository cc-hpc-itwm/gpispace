#include "Job.hpp"

#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
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

    Job::Job(const sdpa::job_id_t id,
                     const sdpa::job_desc_t desc,
                     const sdpa::job_id_t &parent)
        : id_(id)
        , desc_(desc)
        , parent_(parent)
        , type_(Job::WORKER)
        , result_()
        , m_error_code(0)
        , m_error_message()
        , walltime_(2592000) // walltime in seconds: one month by default
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
      return m_error_code;
    }

    std::string const& Job::error_message () const
    {
      return m_error_message;
    }

    Job& Job::error_code(int ec)
    {
      m_error_code = ec;
      return *this;
    }

    Job& Job::error_message(std::string const &msg)
    {
      m_error_message = msg;
      return *this;
    }

    void Job::set_owner(const sdpa::worker_id_t& owner)
    {
      m_owner = owner;
    }

    sdpa::worker_id_t Job::owner()
    {
      return m_owner;
    }

    sdpa::status::code Job::getStatus()
    {
      return state_code (*current_state());
    }

    bool Job::completed()
    {
      return sdpa::status::is_terminal (getStatus());
    }

    bool Job::is_running()
    {
      return sdpa::status::is_running (getStatus());
    }

    bool Job::isMasterJob()
    {
      return type_ == Job::MASTER;
    }

    void Job::setType(const job_type& type)
    {
      type_ = type;
    }

    void Job::setResult(const sdpa::job_result_t& arg_results)
    {
      result_ = arg_results;
    }

    void Job::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
    }

    void Job::action_job_finished(const sdpa::events::JobFinishedEvent& evt/* evt */)
    {
        setResult(evt.result());
    }

    void Job::action_job_failed(const sdpa::events::JobFailedEvent& evt )
    {
        setResult(evt.result());
        error_code(evt.error_code());
        error_message(evt.error_message());
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

    void Job::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);

      sdpa::events::DeleteJobAckEvent::Ptr pDelJobReply(new sdpa::events::DeleteJobAckEvent(pEvt->to(), pEvt->from(), id(), pEvt->id()) );
      ptr_comm->sendEventToMaster(pDelJobReply);
    }

    void Job::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IAgent* pDaemon )
    {
      assert (pDaemon);
      // attention, no action called!
      lock_type const _ (mtx_);
      process_event (*pEvt);

      sdpa::events::JobStatusReplyEvent::Ptr const pStatReply
        (new sdpa::events::JobStatusReplyEvent ( pEvt->to()
                                               , pEvt->from()
                                               , id()
                                               , getStatus()
                                               , error_code()
                                               , error_message()
                                               )
        );

      pDaemon->sendEventToMaster (pStatReply);
    }

    void Job::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
      const sdpa::events::JobResultsReplyEvent::Ptr pResReply( new sdpa::events::JobResultsReplyEvent( pEvt->to(), pEvt->from(), id(), result() ));
      ptr_comm->sendEventToMaster(pResReply);
    }

    void Job::Reschedule(sdpa::daemon::Scheduler*  pSched)
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

    std::string Job::print_info()
    {
        std::ostringstream os;
        os<<std::endl;
        os<<"id: "<<id_<<std::endl;
        os<<"type: "<<type_<<std::endl;
        os<<"status: "<<getStatus()<<std::endl;
        os<<"parent: "<<parent_<<std::endl;
        os<<"error-code: " << m_error_code << std::endl;
        os<<"error-message: \"" << m_error_message << "\"" << std::endl;
        //os<<"description: "<<desc_<<std::endl;

        return os.str();
    }
}}
