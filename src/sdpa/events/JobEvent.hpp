// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/types.hpp>

#include <string>

namespace sdpa
{
  namespace events
  {
    class JobEvent : public sdpa::events::SDPAEvent
    {
    public:
      JobEvent (sdpa::job_id_t const& a_job_id)
        : SDPAEvent()
        , job_id_ (a_job_id)
      {}

      sdpa::job_id_t const& job_id() const
      {
        return job_id_;
      }

    private:
      sdpa::job_id_t job_id_;
    };
  }
}
