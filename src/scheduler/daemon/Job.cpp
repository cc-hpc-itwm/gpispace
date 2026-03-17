// Copyright (C) 2010,2013-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/private/scheduler_types_implementation.hpp>
#include <gspc/drts/scheduler_types.hpp>
#include <gspc/scheduler/daemon/GetSchedulerType.hpp>
#include <gspc/scheduler/daemon/Job.hpp>

#include <gspc/util/make_optional.hpp>


  namespace gspc::scheduler::daemon
  {
    Job::Job ( job_id_t id
             , gspc::we::type::Activity activity
             , job_source source
             , job_handler handler
             , gspc::we::type::Requirements_and_preferences
                 requirements_and_preferences
             )
      : _activity (std::move (activity))
      , id_ (id)
      , _source (std::move (source))
      , _handler (std::move (handler))
      , _requirements_and_preferences (std::move (requirements_and_preferences))
      , _scheduler_type
          ( FHG_UTIL_MAKE_OPTIONAL
              ( std::holds_alternative<job_source_client> (_source)
              , get_scheduler_type (_activity)
              )
          )
      , m_error_message()
      , result_()
    {
      start();
    }

    job_id_t const& Job::id() const
    {
      return id_;
    }
    job_source const& Job::source() const
    {
      return _source;
    }
    gspc::we::type::Requirements_and_preferences
      Job::requirements_and_preferences() const
    {
      return _requirements_and_preferences;
    }

    std::optional<gspc::scheduler::Type> const& Job::scheduler_type() const
    {
      return _scheduler_type;
    }

    std::string Job::error_message () const
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      return m_error_message;
    }
    gspc::we::type::Activity const& Job::result() const
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
    void Job::JobFinished (gspc::we::type::Activity result)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_finished());
      result_ = std::move (result);
    }
    void Job::Reschedule()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      process_event (e_reschedule());
    }

    bool Job::check_and_inc_retry_counter()
    {
      return !_requirements_and_preferences.maximum_number_of_retries()
        || _retry_counter++ < _requirements_and_preferences.maximum_number_of_retries().value();
    }
  }
