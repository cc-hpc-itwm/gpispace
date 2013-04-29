// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CPP_GET_HPP
#define _WE_TYPE_VALUE_CPP_GET_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>

#include <stdexcept>

namespace value
{
  namespace cpp
  {
    namespace exception
    {
      class bad_get : public std::runtime_error
      {
      public:
        bad_get (const std::string & msg)
          : std::runtime_error ("bad_get: " + msg)
        {}
      };
    }

    // ********************************************************************* //

    class get_level : public boost::static_visitor<const type &>
    {
    private:
      const std::string & name;

    public:
      get_level (const std::string & _name) : name (_name) {}

      const type & operator () (const structured_t & s) const
      {
        map_type::const_iterator field (s.map().find (name));

        if (field == s.map().end())
          {
            throw exception::bad_get ("missing field: " + name);
          }

        return field->second;
      }

      const type & operator () (const literal::type &) const
      {
        throw exception::bad_get ("cannot get field "+name+" from a literal");
      }
    };

    // ********************************************************************* //

    template <typename T>
    class get : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator () (const literal::type & literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator () (const value::structured_t &) const
      {
        throw exception::bad_get ("cannot get literal from structured value");
      }
    };

    // ********************************************************************* //
  }

  template <typename T>
  typename cpp::get<T const &>::result_type
  get (const type & v)
  {
    return boost::apply_visitor (cpp::get<T const &>(), v);
  }

  inline const type &
  get_level (const std::string & field, const type & v)
  {
    return boost::apply_visitor (cpp::get_level (field), v);
  }
}

#endif
