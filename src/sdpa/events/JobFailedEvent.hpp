// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
