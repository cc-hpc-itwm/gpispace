// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/line_by_line.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class echo : private line_by_line, public std::ostream
      {
      public:
        echo (std::ostream& os = std::cout);
      };
    }
  }
}
