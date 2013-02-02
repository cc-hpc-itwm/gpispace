// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_HASH_HPP
#define _WE_TYPE_VALUE_HASH_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>

#include <boost/functional/hash.hpp>

namespace value
{
  namespace visitor
  {
    //this is *not* O(1), but safe
    class hash : public boost::static_visitor<std::size_t>
    {
    public:
      std::size_t operator () (const literal::type & v) const
      {
        return boost::hash<literal::type>()(v);
      }

      std::size_t operator () (const structured_t & map) const
      {
        std::size_t v (0);

        for ( structured_t::map_t::const_iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
          {
            std::size_t hash_field (0);

            boost::hash_combine (hash_field, pos->first);

            boost::hash_combine ( hash_field
                                , boost::apply_visitor (hash(), pos->second)
                                );

            v += hash_field;
          }

        return v;
      }
    };
  }
}

#endif
