// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_EQ_HPP
#define _WE_TYPE_VALUE_EQ_HPP

#include <we/type/value.hpp>

#include <we/type/literal.hpp>

#include <algorithm>

namespace value
{
  namespace visitor
  {
    class eq : public boost::static_visitor<bool>
    {
    public:
      bool operator () (const literal::type & x, const literal::type & y) const
      {
        return x == y;
      }

      bool operator () (const structured_t & x, const structured_t & y) const
      {
        structured_t::const_iterator pos_x (x.begin());
        structured_t::const_iterator pos_y (y.begin());
        const structured_t::const_iterator end_x (x.end());

        bool all_eq (  std::distance(pos_x, end_x)
                    == std::distance(pos_y, y.end())
                    );

        for ( ; all_eq && pos_x != end_x; ++pos_x, ++pos_y)
          {
            all_eq =
              (pos_x->first == pos_y->first)
              &&
              boost::apply_visitor (eq(), pos_x->second, pos_y->second);
          }

        return all_eq;
      }

      template<typename A, typename B>
      bool operator () (const A &, const B &) const
      {
        return false;
      }
    };
  }
}

#endif
