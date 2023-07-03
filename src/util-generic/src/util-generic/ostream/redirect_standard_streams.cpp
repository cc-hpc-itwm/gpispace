// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
