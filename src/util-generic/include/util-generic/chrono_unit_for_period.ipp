// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>

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
