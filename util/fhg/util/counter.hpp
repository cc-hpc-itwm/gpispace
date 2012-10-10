// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_UTIL_COUNTER_HPP
#define _FHG_UTIL_COUNTER_HPP 1

#include <boost/noncopyable.hpp>

namespace fhg
{
  namespace util
  {
    //! Always incrementing counter with templated value type. Can be
    //! cast to value type for fast access to new number.
    template<typename VALUE_TYPE>
    class counter : private boost::noncopyable
    {
    public:
      typedef VALUE_TYPE value_type;

      counter()
        : _value (0)
      { }
      counter (const value_type& initial_value)
        : _value (initial_value)
      { }

      value_type next()
      {
        return _value++;
      }
      operator value_type()
      {
        return next();
      }

    private:
      value_type _value;
    };
  }
}

#endif
