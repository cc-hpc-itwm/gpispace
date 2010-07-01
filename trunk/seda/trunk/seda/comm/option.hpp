#ifndef SEDA_COMM_OPTION_HPP
#define SEDA_COMM_OPTION_HPP 1

namespace seda
{
  namespace comm
  {
    namespace option
    {
      namespace detail
      {
        template <typename T>
        struct option
        {
          typedef T value_type;
          typedef option<T> super;

          option (T v)
            : value (v)
          { }

          operator T () const
          {
            return value;
          }

          T value;
        };
      }
    }
  }
}

#endif
