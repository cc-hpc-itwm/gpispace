// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/namespace.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace ns
      {
        open::open ( fhg::util::indenter& indent
                   , ::boost::optional<std::string> const& tag
                   )
          : _indent (indent)
          , _tag (tag)
        {}

        open::open (fhg::util::indenter& indent, std::string const& tag)
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          os << _indent << "namespace";
          if (_tag)
          {
            os << " " << *_tag;
          }
          return os << block::open (_indent);
        }

        close::close (fhg::util::indenter& indent)
          : _indent (indent)
        {}
        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << block::close (_indent);
        }
      }
    }
  }
}
