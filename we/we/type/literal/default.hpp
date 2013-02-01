// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_DEFAULT_HPP
#define _WE_TYPE_LITERAL_DEFAULT_HPP

#include <boost/unordered_map.hpp>

#include <we/type/signature.hpp>
#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <we/type/literal/name.hpp>

namespace literal
{
  typedef boost::unordered_map<std::string, type> default_t;

  inline default_t init_default (void)
  {
    default_t dflt;

    dflt[CONTROL()] = we::type::literal::control();
    dflt[BOOL()] = bool();
    dflt[LONG()] = long();
    dflt[DOUBLE()] = double();
    dflt[CHAR()] = char();
    dflt[STRING()] = std::string();
    dflt[BITSET()] = bitsetofint::type();
    dflt[STACK()] = literal::stack_type();
    dflt[MAP()] = literal::map_type();
    dflt[SET()] = literal::set_type();
    dflt[BYTEARRAY()] = bytearray::type();

    return dflt;
  }

  inline type of_type (const type_name_t & type_name)
  {
    static const default_t dflt (init_default());

    default_t::const_iterator pos (dflt.find (type_name));

    if (pos == dflt.end())
      {
        throw std::runtime_error
          ("literal::of_type: requested default value for unknown type "
          + type_name
          );
      }

    return pos->second;
  }
}

#endif
