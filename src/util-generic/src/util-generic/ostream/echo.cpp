// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/echo.hpp>

#include <util-generic/ostream/put_time.hpp>

#include <chrono>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      echo::echo (std::ostream& os)
        : line_by_line
            ( [&os] (std::string const& line)
              {
                os << '[' << put_time<std::chrono::system_clock>() << ']'
                   << ' ' << line
                   << std::endl;
              }
            )
        , std::ostream (this)
      {}
    }
  }
}
