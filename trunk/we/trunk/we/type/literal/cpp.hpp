// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_CPP_HPP
#define _WE_TYPE_LITERAL_CPP_HPP

#include <we/type/literal/name.hpp>

#include <boost/unordered_map.hpp>

namespace literal
{
  namespace cpp
  {
    struct name
    {
    public:
      typedef boost::unordered_map<type_name_t, std::string> map_type;

    private:
      map_type m;

    public:
      name (void) : m ()
      {
        m[literal::CONTROL] = "control";
        m[literal::BOOL]    = "bool";
        m[literal::LONG]    = "long";
        m[literal::DOUBLE]  = "double";
        m[literal::CHAR]    = "char";
        m[literal::STRING]  = "std::string";
        m[literal::BITSET]  = "bitsetofint::type";
      }

      const map_type::mapped_type & translate (const type_name_t & t) const
      {
        return m.at (t);
      }
    };

    inline const name::map_type::mapped_type & translate (const type_name_t & t)
    {
      static name n;

      return n.translate (t);
    }
  }
}

#endif
