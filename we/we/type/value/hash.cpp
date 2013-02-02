// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/hash.hpp>

#include <we/type/literal.hpp>

#include <boost/functional/hash.hpp>
#include <boost/foreach.hpp>

namespace value
{
  namespace
  {
    //this is *not* O(1), but safe
    class visitor_hash : public boost::static_visitor<std::size_t>
    {
    public:
      std::size_t operator () (const literal::type & v) const
      {
        return boost::hash<literal::type>()(v);
      }

      std::size_t operator () (const structured_t & map) const
      {
        std::size_t v (0);

        typedef std::pair<signature::field_name_t, type> st_type;

        BOOST_FOREACH (const st_type& st, map)
        {
          std::size_t hash_field (0);

          boost::hash_combine (hash_field, st.first);

          boost::hash_combine ( hash_field
                              , boost::apply_visitor (*this, st.second)
                              );

          v += hash_field;
        }

        return v;
      }
    };
  }

  std::size_t hash_value (const type& x)
  {
    return boost::apply_visitor (visitor_hash(), x);
  }
}
