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

#include <util-generic/chrono_unit_for_period.hpp>

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      template<typename Duration, typename Clock>
        scoped<Duration, Clock>::scoped ( std::string description
                                        , std::ostream& os
                                        )
          : _os (os)
          , _description (std::move (description))
          , _start (Clock::now())
      {
        _os << "START: " << _description << "...\n";
      }

      template<typename Duration, typename Clock>
        scoped<Duration, Clock>::~scoped()
      {
        auto const duration
          (std::chrono::duration_cast<Duration> (Clock::now() - _start));

        _os << "DONE: " << _description
            << " [" << duration.count()
            << ' '
            << fhg::util::chrono_unit_for_period<typename Duration::period>()
            << "]\n"
          ;
      }
    }
  }
}
