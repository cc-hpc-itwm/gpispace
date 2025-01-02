// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cpp/include.hpp>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      include::include (std::string const& fname)
        : _fname (fname)
      {}
      std::ostream& include::operator() (std::ostream& os) const
      {
        return os << "#include <" << _fname << ">" << std::endl;
      }
    }
  }
}
