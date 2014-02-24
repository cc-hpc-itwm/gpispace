#include <sdpa/daemon/Job.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    Job::Job ( const job_id_t id
             , const job_desc_t desc
             , bool is_master_job
             , const worker_id_t& owner
             )
      : desc_ (desc)
      , id_ (id)
      , _is_master_job (is_master_job)
      , m_owner (owner)
      , m_error_message()
      , result_()
    {
      start();
    }

    const job_desc_t & Job::description() const
    {
      return desc_;
    }
    const job_id_t & Job::id() const
    {
      return id_;
    }
    bool Job::isMasterJob() const
    {
      return _is_master_job;
    }
    worker_id_t Job::owner() const
    {
      return m_owner;
    }

    std::string Job::error_message () const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return m_error_message;
    }
    const job_result_t& Job::result() const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return result_;
    }

    status::code Job::getStatus() const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return state_code (*current_state());
    }

    void Job::CancelJob()
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_begin_cancel());
    }
    void Job::CancelJobAck()
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_canceled());
    }
    void Job::Dispatch()
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_dispatch());
    }
    void Job::JobFailed (std::string error_message)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_failed());
      m_error_message = error_message;
    }
    void Job::JobFinished (job_result_t result)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_finished());
      result_ = result;
    }
    void Job::Reschedule()
    {
      boost::mutex::scoped_lock const _ (mtx_);
      process_event (e_reschedule());
    }
  }
}
