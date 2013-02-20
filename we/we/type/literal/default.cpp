// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/default.hpp>

#include <boost/unordered_map.hpp>

#include <we/type/signature.hpp>
#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <we/type/literal/name.hpp>

namespace literal
{
  namespace
  {
    typedef boost::unordered_map<std::string, type> default_t;

    default_t init_default()
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
  }

  type of_type (const std::string& type_name)
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
