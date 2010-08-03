// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_SHOW_HPP
#define _WE_TYPE_VALUE_SHOW_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>

#include <iostream>

namespace value
{
  std::ostream & operator << (std::ostream &, const type &);

  namespace visitor
  {
    class show : public boost::static_visitor<std::ostream &>
    {
    private:
      std::ostream & s;

    public:
      show (std::ostream & _s) : s(_s) {}

      std::ostream & operator () (const literal::type & v) const
      {
        return s << literal::show (v);
      }

      std::ostream & operator () (const structured_t & map) const
      {
        s << "[";

        for ( structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          s << ((field != map.begin()) ? ", " : "")
            << field->first << " := " << field->second
            ;

        s << "]";

        return s;
      }
    };
  }

  inline std::ostream & operator << (std::ostream & s, const type & x)
  {
    return boost::apply_visitor (visitor::show (s), x);
  }
}

#endif
