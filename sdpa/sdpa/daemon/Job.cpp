#include <sdpa/daemon/Job.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
  namespace daemon {
    Job::Job( const job_id_t id
              , const job_desc_t desc
              , bool is_master_job
              , const worker_id_t& owner
            )
        : id_(id)
        , desc_(desc)
        , _is_master_job (is_master_job)
        , result_()
        , m_error_message()
        , m_owner(owner)
    {
      start();
    }

    const job_id_t & Job::id() const
    {
      return id_;
    }

    const job_desc_t & Job::description() const
    {
      return desc_;
    }

    const job_result_t& Job::result() const
    {
      return result_;
    }

    std::string Job::error_message () const
    {
      lock_type lock(mtx_);
      return m_error_message;
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

    bool Job::isMasterJob() const
    {
      return _is_master_job;
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
      m_error_message = pEvt->error_message();
    }

    void Job::JobFinished(const events::JobFinishedEvent* pEvt)
    {
      lock_type lock(mtx_);
      process_event(*pEvt);
      result_ = pEvt->result();
    }

    void Job::Reschedule()
    {
      lock_type lock(mtx_);
      process_event(MSMRescheduleEvent());
    }

    void Job::Dispatch()
    {
      lock_type lock(mtx_);
      process_event (MSMDispatchJobEvent (NULL));
    }
}}
