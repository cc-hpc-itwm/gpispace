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

#include <chrono>
#include <string>

namespace fhg
{
  namespace util
  {
    template<typename Period>
      std::string chrono_unit_for_period()
    {
      if (Period::den == 1)
      {
        return "[" + std::to_string (Period::num) + "]s";
      }
      else
      {
        return "[" + std::to_string (Period::num) + "/"
          + std::to_string (Period::den) + "]s";
      }
    }

#define CHRONO_UNIT_FOR_PREDEFINED_PERIOD(ratio_)               \
    template<> std::string chrono_unit_for_period<ratio_>()

    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::nanoseconds::period);
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::microseconds::period);
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::milliseconds::period);
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::seconds::period);
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::minutes::period);
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (std::chrono::hours::period);

#undef CHRONO_UNIT_FOR_PREDEFINED_PERIOD
  }
}
