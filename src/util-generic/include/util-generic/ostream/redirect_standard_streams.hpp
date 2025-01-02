// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/redirect.hpp>

#include <mutex>
#include <ostream>
#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class redirect_standard_streams
      {
      public:
        redirect_standard_streams (std::vector<std::string>& lines);

      private:
        std::mutex _guard;
        std::vector<std::string>& _lines;
        struct appender : public redirect
        {
          appender ( std::ostream& os
                   , std::string prefix
                   , std::vector<std::string>& lines
                   , std::mutex& guard
                   );
          prepend_line _prepender;
        };
        appender _append_from_clog;
        appender _append_from_cout;
        appender _append_from_cerr;
      };
    }
  }
}
