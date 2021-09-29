// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/daemon/Job.hpp>

namespace sdpa
{
  namespace daemon
  {
    Job::Job ( job_id_t id
             , we::type::Activity activity
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

    job_id_t const& Job::id() const
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
    we::type::Activity const& Job::result() const
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
    void Job::JobFinished (we::type::Activity result)
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
  }
}
