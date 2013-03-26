// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/show.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>

#include <iostream>

namespace value
{
  namespace
  {
    class visitor_show : public boost::static_visitor<std::ostream&>
    {
    private:
      std::ostream& s;

    public:
      visitor_show (std::ostream& _s) : s(_s) {}

      std::ostream& operator() (const literal::type& v) const
      {
        return s << literal::show (v);
      }

      std::ostream& operator() (const structured_t& map) const
      {
        s << "[";

        for ( map_type::const_iterator field (map.map().begin())
            ; field != map.map().end()
            ; ++field
            )
          s << ((field != map.map().begin()) ? ", " : "")
            << field->first << " := " << field->second
            ;

        s << "]";

        return s;
      }
    };
  }

  std::ostream& operator << (std::ostream& s, const type& x)
  {
    return boost::apply_visitor (visitor_show (s), x);
  }
}
