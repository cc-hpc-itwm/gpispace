// mirko.rahn@itwm.fhg.de

#include <we2/type/compat.hpp>

#include <boost/foreach.hpp>

#include <algorithm>
#include <stdexcept>

namespace pnet
{
  namespace type
  {
    namespace compat
    {
      namespace
      {
        class we22we : public boost::static_visitor< ::value::type>
        {
        public:
          ::value::type operator() (const we::type::literal::control& c) const
          {
            return c;
          }
          ::value::type operator() (const bool& b) const
          {
            return b;
          }
          ::value::type operator() (const int&) const
          {
            throw std::runtime_error ("compat: <int>");
          }
          ::value::type operator() (const long& l) const
          {
            return l;
          }
          ::value::type operator() (const unsigned int&) const
          {
            throw std::runtime_error ("compat: <unsigned int>");
          }
          ::value::type operator() (const unsigned long&) const
          {
            throw std::runtime_error ("compat: <unsigned long>");
          }
          ::value::type operator() (const float&) const
          {
            throw std::runtime_error ("compat: <float>");
          }
          ::value::type operator() (const double& d) const
          {
            return d;
          }
          ::value::type operator() (const char& c) const
          {
            return c;
          }
          ::value::type operator() (const std::string& s) const
          {
            return s;
          }
          ::value::type operator() (const bitsetofint::type& bs) const
          {
            return bs;
          }
          ::value::type operator() (const bytearray::type& ba) const
          {
            return ba;
          }
          ::value::type operator() (const std::list<value::value_type>& l) const
          {
            ::literal::stack_type s;
            BOOST_FOREACH (const value::value_type& v, l)
            {
              s.push_back (boost::get<long> (v));
            }
            return s;
          }
          ::value::type operator() (const std::set<value::value_type>& s2) const
          {
            ::literal::set_type s;
            BOOST_FOREACH (const value::value_type& v, s2)
            {
              s.insert (boost::get<long> (v));
            }
            return s;
          }
          ::value::type operator() (const std::map< value::value_type
                                                  , value::value_type
                                                  >& m2) const
          {
            ::literal::map_type m;
            typedef std::map<value::value_type, value::value_type> map_type;
            BOOST_FOREACH (const map_type::value_type& kv, m2)
            {
              m.insert (std::make_pair ( boost::get<long> (kv.first)
                                       , boost::get<long> (kv.second)
                                       )
                       );
            }
            return m;
          }
          ::value::type operator() (const value::structured_type& m) const
          {
            ::value::structured_t s;
            BOOST_FOREACH (const value::structured_type::value_type& kv, m)
            {
              s[kv.first] = boost::apply_visitor (*this, kv.second);
            }
            return s;
          }
        };
      }

      ::value::type COMPAT (const value::value_type& v)
      {
        return boost::apply_visitor (we22we(), v);
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
            BOOST_FOREACH (const long& x, s)
            {
              l.push_back (value::value_type (x));
            }
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

      namespace
      {
        class field : public boost::static_visitor<signature::field_type>
        {
        public:
          field (const std::string& name)
            : _name (name)
          {}

          signature::field_type operator() (const std::string& t) const
          {
            return std::make_pair (_name, t);
          }
          signature::field_type operator()
            (const ::signature::structured_t& s) const
          {
            signature::structure_type s2;

            for ( ::signature::structured_t::const_iterator pos (s.begin())
                ; pos != s.end()
                ; ++pos
                )
            {
              s2.push_back (boost::apply_visitor ( field (pos->first)
                                                 , pos->second
                                                 )
                           );
            }

            return signature::structured_type (std::make_pair (_name, s2));
          }
        private:
          const std::string& _name;
        };

        class sig2sig2
          : public boost::static_visitor<signature::signature_type>
        {
        public:
          sig2sig2 (const ::signature::type& sig)
            : _sig (sig)
          {}

          signature::signature_type operator() (const std::string& t) const
          {
            return t;
          }
          signature::signature_type operator()
            (const ::signature::structured_t& s) const
          {
            signature::structure_type s2;

            for ( ::signature::structured_t::const_iterator pos (s.begin())
                ; pos != s.end()
                ; ++pos
                )
            {
              s2.push_back (boost::apply_visitor ( field (pos->first)
                                                 , pos->second
                                                 )
                           );
            }

            return signature::structured_type (std::make_pair ( _sig.nice()
                                                              , s2
                                                              )
                                              );
          }
        private:
          const ::signature::type& _sig;
        };
      }

      signature::signature_type COMPAT (const ::signature::type& x)
      {
        return boost::apply_visitor (sig2sig2 (x), x.desc());
      }
    }
  }
}
