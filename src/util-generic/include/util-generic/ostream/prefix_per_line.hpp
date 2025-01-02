// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/line_by_line.hpp>

#include <iostream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class prefix_per_line : private line_by_line, public std::ostream
      {
      public:
        prefix_per_line (std::string, std::ostream& os = std::cout);
      };
    }
  }
}
