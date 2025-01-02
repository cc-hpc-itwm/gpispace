// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

        std::ostream& operator() (std::ostream& os) const override
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
