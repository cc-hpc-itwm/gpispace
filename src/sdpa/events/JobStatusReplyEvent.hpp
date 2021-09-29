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

#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>
#include <sdpa/job_states.hpp>

namespace sdpa
{
  namespace events
  {
    class JobStatusReplyEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobStatusReplyEvent> Ptr;

      JobStatusReplyEvent ( sdpa::job_id_t const& a_job_id
                          , sdpa::status::code const& a_status
                          , std::string const& error_message
                          )
        : sdpa::events::JobEvent (a_job_id)
        , status_ (a_status)
        , m_error_message (error_message)
      { }

      sdpa::status::code status() const
      {
        return status_;
      }
      std::string const& error_message() const
      {
        return m_error_message;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobStatusReplyEvent (source, this);
      }

    private:
      sdpa::status::code status_;
      std::string m_error_message;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobStatusReplyEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE_WITH_TEMPORARY (sdpa::status::code, e->status())
      SAVE_TO_ARCHIVE (e->error_message());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobStatusReplyEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (sdpa::status::code, status);
      LOAD_FROM_ARCHIVE (std::string, error_message);

      ::new (e) JobStatusReplyEvent (job_id, status, error_message);
    }
  }
}
