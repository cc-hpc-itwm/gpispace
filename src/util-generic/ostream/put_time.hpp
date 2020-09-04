// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/ostream/modifier.hpp>

#include <chrono>
#include <ostream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      template<typename Clock, typename Duration = typename Clock::duration>
        struct put_time : public fhg::util::ostream::modifier
      {
        put_time()
          : _time_point (Clock::now())
        {}
        put_time (std::chrono::time_point<Clock, Duration> const& time_point)
          : _time_point (time_point)
        {}

        virtual std::ostream& operator() (std::ostream& os) const override
        {
          std::time_t const tp_c (Clock::to_time_t (_time_point));

          //! \todo use std::put_time once it is supported
          char buf[100];
          std::strftime (buf, sizeof (buf), "%F %T", std::localtime (&tp_c));

          return os << std::string (buf);
        }

        std::chrono::time_point<Clock, Duration> const _time_point;
      };
    }
  }
}
