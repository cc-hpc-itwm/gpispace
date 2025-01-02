// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/parse/error.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        expected::expected (std::string const& what, position const& inp)
          : generic {fmt::format ("expected '{}'", what), inp}
        {}
      }
    }
  }
}
