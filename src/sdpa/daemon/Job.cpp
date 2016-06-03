#include <sdpa/daemon/Job.hpp>

namespace sdpa
{
  namespace daemon
  {
    Job::Job ( const job_id_t id
             , we::type::activity_t activity
             , job_source source
             , job_handler handler
             , job_requirements_t requirements
             )
      : _activity (std::move (activity))
      , id_ (id)
      , _source (std::move (source))
      , _handler (std::move (handler))
      , _requirements (std::move (requirements))
      , m_error_message()
      , result_()
    {
      start();
    }

    const job_id_t & Job::id() const
    {
      return id_;
    }
    job_source const& Job::source() const
    {
      return _source;
    }
    job_requirements_t Job::requirements() const
    {
      return _requirements;
    }

    std::string Job::error_message () const
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      return m_error_message;
    }
    const we::type::activity_t& Job::result() const
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      return result_;
    }

    status::code Job::getStatus() const
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      return state_code (*current_state());
    }

    void Job::CancelJob()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_begin_cancel());
    }
    void Job::CancelJobAck()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_canceled());
    }
    void Job::Dispatch()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_dispatch());
    }
    void Job::JobFailed (std::string error_message)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_failed());
      m_error_message = error_message;
    }
    void Job::JobFinished (we::type::activity_t result)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_finished());
      result_ = result;
    }
    void Job::Reschedule()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_reschedule());
    }
  }
}
