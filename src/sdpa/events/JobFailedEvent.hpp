// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

namespace sdpa
{
  namespace events
  {
    class JobFailedEvent : public JobEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<JobFailedEvent>;

      JobFailedEvent ( sdpa::job_id_t const& a_job_id
                     , std::string error_message
                     )
        : sdpa::events::JobEvent (a_job_id)
        , m_error_message (error_message)
      {}

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFailedEvent (source, this);
      }

      std::string const& error_message() const
      {
        return m_error_message;
      }

    private:
      std::string m_error_message;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->error_message());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (std::string, error_message);

      ::new (e) JobFailedEvent (job_id, error_message);
    }
  }
}
