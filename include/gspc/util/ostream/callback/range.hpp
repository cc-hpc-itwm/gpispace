#pragma once

#include <gspc/util/first_then.hpp>
#include <gspc/util/ostream/callback/open.hpp>
#include <gspc/util/ostream/to_string.hpp>




      namespace gspc::util::ostream::callback
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
