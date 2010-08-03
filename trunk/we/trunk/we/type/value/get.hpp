// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_GET_HPP
#define _WE_TYPE_VALUE_GET_HPP

#include <we/type/value.hpp>

#include <we/type/signature.hpp>
#include <we/type/literal.hpp>

#include <sstream>
#include <stdexcept>

namespace value
{
  namespace visitor
  {
    class get_field : public boost::static_visitor<const type &>
    {
    private:
      signature::field_name_t name;

    public:
      get_field (const signature::field_name_t & _name) : name (_name) {}

      const type & operator () (const structured_t & s) const
      {
        structured_t::const_iterator pos (s.find (name));

        if (pos == s.end())
          throw std::runtime_error ("missing field " + name);

        return pos->second;
      }

      const type & operator () (const literal::type & l) const
      {
        std::ostringstream s;

        s << "cannot get field " << name << " from the literal " << l;

        throw std::runtime_error (s.str());
      }
    };

    template <typename T>
    class get_literal_value : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator () (const literal::type & literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator () (const value::structured_t & o) const
      {
        std::ostringstream s;

        s << "bad get: expected literal, got: " << o;

        throw std::runtime_error (s.str());
      }
    };
  }

  template <typename T, typename V>
  typename visitor::get_literal_value<T const &>::result_type
  get_literal_value (const V & v)
  {
    return boost::apply_visitor (visitor::get_literal_value<T const &>(), v);
  }

  inline const type &
  get_field (const signature::field_name_t & field, const type & v)
  {
    return boost::apply_visitor (visitor::get_field (field), v);
  }
}

#endif
