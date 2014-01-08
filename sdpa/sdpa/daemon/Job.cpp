#include "Job.hpp"

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
  namespace daemon {
    Job::Job( const job_id_t id
              , const job_desc_t desc
              , const boost::optional<job_id_t> &parent
              , bool is_master_job
              , const worker_id_t& owner
            )
        : id_(id)
        , desc_(desc)
        , parent_(parent)
        , _is_master_job (is_master_job)
        , result_()
        , m_error_code(0)
        , m_error_message()
        , m_owner(owner)
    {
      start();
    }

    const job_id_t & Job::id() const
    {
      return id_;
    }

    const boost::optional<job_id_t> & Job::parent() const
    {
      return parent_;
    }

    const job_desc_t & Job::description() const
    {
      return desc_;
    }

    const job_result_t& Job::result() const
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

    void Job::set_owner(const worker_id_t& owner)
    {
      m_owner = owner;
    }

    worker_id_t Job::owner() const
    {
      return m_owner;
    }

    status::code Job::getStatus() const
    {
      lock_type lock(mtx_);
      return state_code (*current_state());
    }

    bool Job::completed() const
    {
      return status::is_terminal (getStatus());
    }

    bool Job::is_running() const
    {
      return status::is_running (getStatus());
    }

    bool Job::is_canceled() const
    {
      return status::is_canceled (getStatus());
    }

    bool Job::isMasterJob() const
    {
      return _is_master_job;
    }

    void Job::action_job_finished(const events::JobFinishedEvent& evt/* evt */)
    {
      lock_type lock(mtx_);
      result_ = evt.result();
    }

    void Job::action_job_failed(const events::JobFailedEvent& evt )
    {
      lock_type lock(mtx_);
      m_error_code = evt.error_code();
      m_error_message = evt.error_message();
    }

    void Job::action_reschedule_job(const MSMRescheduleEvent& evt)
    {
      DLOG(TRACE, "Reschedule the job "<<id());
      evt.ptrScheduler()->schedule(id());
    }

    void Job::action_retrieve_job_results(const MSMRetrieveJobResultsEvent& e)
    {
      const events::JobResultsReplyEvent::Ptr pResReply(
         new events::JobResultsReplyEvent( e.from()
                                                 , e.to()
                                                 , id()
                                                 , result() ));
      e.ptrAgent()->sendEventToOther(pResReply);
    }

    //transitions
    void Job::CancelJob(const events::CancelJobEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::CancelJobAck(const events::CancelJobAckEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::JobFailed(const events::JobFailedEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::JobFinished(const events::JobFinishedEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
    }

    void Job::RetrieveJobResults(const events::RetrieveJobResultsEvent* pEvt, GenericDaemon* pAgent)
    {
      lock_type lock(mtx_);
      process_event(MSMRetrieveJobResultsEvent(pEvt, pAgent));
    }

    void Job::Reschedule(SchedulerBase*  pSched)
    {
      lock_type lock(mtx_);
      process_event(MSMRescheduleEvent (pSched));
    }

    void Job::Dispatch()
    {
      lock_type lock(mtx_);
      process_event (MSMResumeJobEvent (NULL));
    }
}}
