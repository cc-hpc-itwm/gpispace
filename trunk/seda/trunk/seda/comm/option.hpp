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

          void set (T v)
          {
            value = v;
          }

          // doesn't work as I expected ;-(
          // virtual derived_type & operator=(T rhs)
          // {
          //   this->value = rhs;
          //   return static_cast<derived_type&>(*this);
          // }

        protected:
          T value;
        };
      }
    }
  }
}

#endif
