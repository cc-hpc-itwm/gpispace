// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

              auto& pos (range.first);

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
