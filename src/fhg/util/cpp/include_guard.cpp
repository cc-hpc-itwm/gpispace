// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cpp/include_guard.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace include_guard
      {
        open::open (std::string const& name)
          : _name (name)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          return os << "#ifndef _" << _name << std::endl
                    << "#define _" << _name << std::endl
                    << std::endl;
        }

        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << "#endif" << std::endl;
        }
      }
    }
  }
}
