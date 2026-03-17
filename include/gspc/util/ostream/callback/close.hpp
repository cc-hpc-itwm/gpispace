#pragma once

#include <gspc/util/ostream/callback/function.hpp>
#include <gspc/util/ostream/callback/print.hpp>




      namespace gspc::util::ostream::callback
      {
        template<typename T, typename Close>
          constexpr print_function<T> close ( Close close
                                            , print_function<T> f = id<T>()
                                            )
        {
          return [f, close] (std::ostream& os, T const& x) -> std::ostream&
            {
              return os << print<decltype (x)> (f, x) << close;
            };
        }
      }
