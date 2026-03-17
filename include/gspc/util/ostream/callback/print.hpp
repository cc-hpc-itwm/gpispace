#pragma once

#include <gspc/util/ostream/callback/function.hpp>
#include <gspc/util/ostream/modifier.hpp>




      namespace gspc::util::ostream::callback
      {
        template<typename T>
          class print : public modifier
        {
        public:
          print (print_function<T> print_, T x)
            : _print (print_)
            , _x (x)
          {}
          std::ostream& operator() (std::ostream& os) const override
          {
            _print (os, _x); return os;
          }
        private:
          print_function<T> const _print;
          T const _x;
        };
      }
