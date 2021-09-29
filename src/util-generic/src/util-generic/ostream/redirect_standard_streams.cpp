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

#include <util-generic/ostream/redirect_standard_streams.hpp>

#include <algorithm>
#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      redirect_standard_streams::redirect_standard_streams
          (std::vector<std::string>& lines)
        : _lines (lines)
        , _append_from_clog (std::clog, "log: ", _lines, _guard)
        , _append_from_cout (std::cout, "out: ", _lines, _guard)
        , _append_from_cerr (std::cerr, "err: ", _lines, _guard)
      {}

      redirect_standard_streams::appender::appender
          ( std::ostream& os
          , std::string prefix
          , std::vector<std::string>& lines
          , std::mutex& guard
          )
        : redirect
            ( os
            , [&lines, &guard] (std::string const& line)
              {
                std::lock_guard<std::mutex> const _ (guard);

                lines.emplace_back (line);
              }
            )
        , _prepender (os, prefix)
      {}
    }
  }
}
