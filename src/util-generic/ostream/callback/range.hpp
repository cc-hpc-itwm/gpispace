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

#pragma once

#include <util-generic/first_then.hpp>
#include <util-generic/ostream/callback/open.hpp>
#include <util-generic/ostream/to_string.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        template<typename Iterator, typename Separator>
          constexpr print_function<std::pair<Iterator, Iterator>>
            range ( Separator separator
                  , print_function<typename std::iterator_traits<Iterator>::value_type> f
                  = id<typename std::iterator_traits<Iterator>::value_type>()
                  )
        {
          return [f, separator] ( std::ostream& os
                                , std::pair<Iterator, Iterator> range
                                ) -> std::ostream&
            {
              first_then<std::string> const sep ("", to_string (separator));

              Iterator pos {range.first};

              while (pos != range.second)
              {
                ostream::callback::open (std::ref (sep), f) (os, *pos);

                ++pos;
              }

              return os;
            };
        }
      }
    }
  }
}
