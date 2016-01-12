#include <sdpa/daemon/Job.hpp>

namespace sdpa
{
  namespace daemon
  {
    Job::Job ( const job_id_t id
             , we::type::activity_t activity
             , boost::optional<master_info_t::iterator> owner
             , job_requirements_t requirements
             )
      : _activity (std::move (activity))
      , id_ (id)
      , m_owner (std::move (owner))
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
    boost::optional<master_info_t::iterator> const& Job::owner() const
    {
      return m_owner;
    }
    job_requirements_t Job::requirements() const
    {
      return _requirements;
    }

    std::string Job::error_message () const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return m_error_message;
    }
    const we::type::activity_t& Job::result() const
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
    void Job::JobFinished (we::type::activity_t result)
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
