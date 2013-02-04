// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/eq.hpp>

#include <we/type/literal.hpp>

namespace value
{
  namespace
  {
    class visitor_eq : public boost::static_visitor<bool>
    {
    public:
      bool operator() (const literal::type& x, const literal::type& y) const
      {
        return x == y;
      }

      bool operator() (const structured_t& x, const structured_t& y) const
      {
        map_type::const_iterator pos_x (x.map().begin());
        map_type::const_iterator pos_y (y.map().begin());
        const map_type::const_iterator end_x (x.map().end());

        bool all_eq (x.map().size() == y.map().size());

        while (all_eq && pos_x != end_x)
          {
            all_eq =
              pos_x->first == pos_y->first
              &&
              boost::apply_visitor (*this, pos_x->second, pos_y->second);

            ++pos_x;
            ++pos_y;
          }

        return all_eq;
      }

      template<typename A, typename B>
      bool operator() (const A&, const B&) const
      {
        return false;
      }
    };
  }

  bool eq (const type& x, const type& y)
  {
    return boost::apply_visitor (visitor_eq(), x, y);
  }
}
