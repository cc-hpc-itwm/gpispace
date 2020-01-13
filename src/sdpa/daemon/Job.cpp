#include <sdpa/daemon/Job.hpp>

namespace sdpa
{
  namespace daemon
  {
    Job::Job ( const job_id_t id
             , we::type::activity_t activity
             , job_source source
             , job_handler handler
             , Requirements_and_preferences requirements_and_preferences
             )
      : _activity (std::move (activity))
      , id_ (id)
      , _source (std::move (source))
      , _handler (std::move (handler))
      , _requirements_and_preferences (std::move (requirements_and_preferences))
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
    Requirements_and_preferences Job::requirements_and_preferences() const
    {
      return _requirements_and_preferences;
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
