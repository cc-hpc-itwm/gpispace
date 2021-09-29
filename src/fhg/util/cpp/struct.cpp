// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <fhg/util/cpp/struct.hpp>
#include <fhg/util/cpp/block.hpp>

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
          , _tag (boost::none)
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
