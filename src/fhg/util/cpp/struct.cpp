// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/struct.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace structure
      {
        open::open (fhg::util::indenter& indent)
          : _indent (indent)
          , _tag (::boost::none)
        {}
        open::open ( fhg::util::indenter& indent
                   , std::string const& tag
                   )
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          os << _indent << "struct";
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
          return os << block::close (_indent) << ";";
        }
      }
    }
  }
}
