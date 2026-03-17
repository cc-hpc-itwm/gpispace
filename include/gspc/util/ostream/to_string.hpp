#pragma once

#include <gspc/util/ostream/callback/function.hpp>
#include <gspc/util/ostream/callback/print.hpp>



    namespace gspc::util::ostream
    {
      template<typename T>
        inline std::string to_string
          ( T x
          , callback::print_function<T> f = callback::id<T>()
          )
      {
        return callback::print<T> (f, x).string();
      }
    }
