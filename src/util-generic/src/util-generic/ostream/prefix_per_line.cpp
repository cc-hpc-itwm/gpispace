// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/prefix_per_line.hpp>

#include <iostream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      prefix_per_line::prefix_per_line (std::string prefix, std::ostream& os)
        : line_by_line
          ( [&os, prefix] (std::string const& line)
            {
              os << prefix << line << std::endl;
            }
          )
        , std::ostream (this)
      {}
    }
  }
}
