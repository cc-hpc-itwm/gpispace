// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/modifier.hpp>

#include <iostream>
#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      std::ostream& operator<< (std::ostream& os, const modifier& m)
      {
        return m (os);
      }
      std::string modifier::string() const
      {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
      }
    }
  }
}
