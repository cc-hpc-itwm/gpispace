// mirko.rahn@itwm.fhg.de

#include <we2/type/compat.hpp>

#include <boost/foreach.hpp>

#include <algorithm>

namespace pnet
{
  namespace type
  {
    namespace compat
    {
      ::value::type COMPAT (const value::value_type&)
      {
        return ::value::type (0L);
      }

      namespace
      {
        class literal2we2 : public boost::static_visitor<value::value_type>
        {
        public:
          value::value_type operator() (const we::type::literal::control& c) const
          {
            return c;
          }
          value::value_type operator() (const bool& b) const
          {
            return b;
          }
          value::value_type operator() (const long& l) const
          {
            return l;
          }
          value::value_type operator() (const double& d) const
          {
            return d;
          }
          value::value_type operator() (const char& c) const
          {
            return c;
          }
          value::value_type operator() (const std::string& s) const
          {
            return s;
          }
          value::value_type operator() (const bitsetofint::type& bs) const
          {
            return bs;
          }
          value::value_type operator() (const ::literal::stack_type& s) const
          {
            std::list<value::value_type> l;
            std::copy (s.begin(), s.end(), l.end());
            return l;
          }
          value::value_type operator() (const ::literal::map_type& m) const
          {
            std::map<value::value_type, value::value_type> m2;
            BOOST_FOREACH (const ::literal::map_type::value_type& kv, m)
            {
              m2.insert (std::make_pair ( value::value_type (kv.first)
                                        , value::value_type (kv.second)
                                        )
                        );
            }
            return m2;
          }
          value::value_type operator() (const ::literal::set_type& s) const
          {
            std::set<value::value_type> s2;
            BOOST_FOREACH (const long& x, s)
            {
              s2.insert (x);
            }
            return s2;
          }
          value::value_type operator() (const ::bytearray::type& ba) const
          {
            return ba;
          }
        };

        class we2we2 : public boost::static_visitor<value::value_type>
        {
        public:
          value::value_type operator() (const ::literal::type& l) const
          {
            return boost::apply_visitor (literal2we2(), l);
          }
          value::value_type operator() (const ::value::structured_t& s) const
          {
            std::list<std::pair<std::string, value::value_type> > s2;
            BOOST_FOREACH (const ::value::map_type::value_type& kv, s.map())
            {
              s2.push_back
                ( std::make_pair ( kv.first
                                 , boost::apply_visitor (*this, kv.second)
                                 )
                );
            }
            return s2;
          }
        };
      }

      value::value_type COMPAT (const ::value::type& x)
      {
        return boost::apply_visitor (we2we2(), x);
      }
    }
  }
}
