// Copyright (C) 2023 Fraunhofer ITWM
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
          : generic (::boost::format ("expected '%1%'") % what, inp)
        {}
      }
    }
  }
}
