// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_GET_HPP
#define _WE_TYPE_VALUE_GET_HPP

#include <we/type/value.hpp>
#include <we/type/signature.hpp>
#include <we/type/literal.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      class bad_get : public std::runtime_error
      {
      public:
        bad_get (const value::structured_t&)
          : std::runtime_error
            ( ( boost::format ("bad get: expexted literal, got %1%") % 1
              ).str()
            )
        {}
      };

      class empty_path : public std::runtime_error
      {
      public:
        empty_path();
      };
    }

    template <typename T>
    class get : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator() (const literal::type& literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator() (const value::structured_t& o) const
      {
        throw exception::bad_get (o);
      }
    };

    template <typename T>
    class get_ref : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator() (literal::type& literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator() (value::structured_t& o) const
      {
        throw exception::bad_get (o);
      }
    };
  }

  template <typename T, typename V>
  const T& get (const V& v)
  {
    return boost::apply_visitor (visitor::get<T const&>(), v);
  }

  template <typename T>
  T& get_ref (type& v)
  {
    return boost::apply_visitor (visitor::get_ref<T&>(), v);
  }
}

#endif
