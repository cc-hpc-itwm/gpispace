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

#include <string>

#include <sdpa/types.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobEvent : public sdpa::events::SDPAEvent
    {
    public:
      JobEvent (const sdpa::job_id_t& a_job_id)
        : SDPAEvent()
        , job_id_ (a_job_id)
      {}

      const sdpa::job_id_t& job_id() const
      {
        return job_id_;
      }

    private:
      sdpa::job_id_t job_id_;
    };
  }
}
