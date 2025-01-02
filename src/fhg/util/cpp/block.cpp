// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cpp/block.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace block
      {
        open::open ( fhg::util::indenter& indent
                   , ::boost::optional<std::string> const& tag
                   )
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          return _tag
            ? os << _indent++ << *_tag << " {"
            : os << _indent++ << "{"
            ;
        }

        close::close (fhg::util::indenter& indent)
          : _indent (indent)
        {}
        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << --_indent << "}";
        }
      }
    }
  }
}
