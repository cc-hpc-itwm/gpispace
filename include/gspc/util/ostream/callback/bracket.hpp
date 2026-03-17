#pragma once

#include <gspc/util/ostream/callback/close.hpp>
#include <gspc/util/ostream/callback/open.hpp>




      namespace gspc::util::ostream::callback
      {
        template<typename T, typename Bracket>
          constexpr print_function<T> bracket ( Bracket o
                                              , Bracket c
                                              , print_function<T> f = id<T>()
                                              )
        {
          return [f, o, c] (std::ostream& os, T const& x) -> std::ostream&
            {
              return open (o, close (c, f)) (os, x);
            };
        }
      }
