#pragma once

#include <gspc/util/ostream/callback/function.hpp>
#include <gspc/util/ostream/callback/print.hpp>




      namespace gspc::util::ostream::callback
      {
        template<typename T, typename Open>
          constexpr print_function<T> open ( Open open
                                           , print_function<T> f = id<T>()
                                           )
        {
          return [f, open] (std::ostream& os, T const& x) -> std::ostream&
            {
              return os << open << print<decltype (x)> (f, x);
            };
        }
      }
