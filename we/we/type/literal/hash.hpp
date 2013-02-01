// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HASH_HPP
#define _WE_TYPE_LITERAL_HASH_HPP

#include <we/type/literal.hpp>

#include <we/type/literal/control.hpp>

#include <boost/functional/hash.hpp>

namespace literal
{
  namespace visitor
  {
    class hash : public boost::static_visitor<std::size_t>
    {
    public:
      std::size_t operator () (const we::type::literal::control &) const
      {
        return 42;
      }

      template<typename T>
      std::size_t operator () (const T & x) const
      {
        boost::hash<T> hasher;

        return hasher(x);
      }
    };
  }
}

namespace boost
{
  static inline std::size_t hash_value (const literal::type & v)
  {
    return boost::apply_visitor (literal::visitor::hash(), v);
  }
}

#endif
