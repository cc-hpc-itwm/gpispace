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
