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
        template <typename T, typename D>
        struct option
        {
          typedef T value_type;
          typedef D derived_type;
          typedef option<T,D> this_type;
          typedef this_type super;

          explicit
          option (T v)
            : value (v)
          { }

          operator T () const
          {
            return value;
          }

          T get (void) const
          {
            return value;
          }

          void set (T v)
          {
            value = v;
          }

        protected:
          T value;
        };
      }
    }
  }
}

#endif
