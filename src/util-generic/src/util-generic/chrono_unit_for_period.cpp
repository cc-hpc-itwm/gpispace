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

#include <util-generic/chrono_unit_for_period.hpp>

namespace fhg
{
  namespace util
  {
#define CHRONO_UNIT_FOR_PREDEFINED_PERIOD(symbol_, ratio_)      \
    template<>                                                  \
      std::string chrono_unit_for_period<ratio_::period>()      \
      {                                                         \
        return #symbol_;                                        \
      }

    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (ns, std::chrono::nanoseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (µs, std::chrono::microseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (ms, std::chrono::milliseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (s, std::chrono::seconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (min, std::chrono::minutes)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (hr, std::chrono::hours)

#undef CHRONO_UNIT_FOR_PREDEFINED_PERIOD
  }
}
