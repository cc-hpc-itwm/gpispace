// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/show.hpp>

#include <iostream>

#include <boost/foreach.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class visitor_show : public boost::static_visitor<std::ostream&>
      {
      public:
        visitor_show (std::ostream& os)
          : _os (os)
        {}

        std::ostream& operator() (const control& c) const
        {
          return _os << "[]";
        }
        std::ostream& operator() (const bool& b) const
        {
          return _os << b;
        }
        std::ostream& operator() (const int& i) const
        {
          return _os << i;
        }
        std::ostream& operator() (const long& i) const
        {
          return _os << i << "L";
        }
        std::ostream& operator() (const unsigned int& i) const
        {
          return _os << i << "U";
        }
        std::ostream& operator() (const unsigned long& i) const
        {
          return _os << i << "UL";
        }
        std::ostream& operator() (const float& f) const
        {
          return _os << f << "f";
        }
        std::ostream& operator() (const double& d) const
        {
          return _os << d;
        }
        std::ostream& operator() (const char& c) const
        {
          return _os << "'" << c << "'";
        }
        std::ostream& operator() (const std::string& s) const
        {
          return _os << "\"" << s << "\"";
        }
        std::ostream& operator() (const bitsetofint::type& bs) const
        {
          return _os << bs;
        }
        std::ostream& operator() (const bytearray::type& ba) const
        {
          return _os << ba;
        }
        std::ostream& operator() (const std::list<value_type>& l) const
        {
          _os << "list (";
          bool first (true);
          BOOST_FOREACH (const value_type& v, l)
            {
              if (!first)
                {
                  _os << ", ";
                }
              boost::apply_visitor (*this, v);
              first = false;
            }
          return _os << ")";
        }
        std::ostream&
        operator() (const std::map<value_type, value_type>& m) const
        {
          _os << "map [";
          bool first (true);
          typedef std::pair<value_type, value_type> kv_type;
          BOOST_FOREACH (const kv_type& kv, m)
            {
              if (!first)
                {
                  _os << ", ";
                }
              boost::apply_visitor (*this, kv.first);
              _os << " => ";
              boost::apply_visitor (*this, kv.second);
              first = false;
            }
          return _os << "]";
        }
        std::ostream& operator() (const std::vector<value_type>& v) const
        {
          _os << "vector (";
          bool first (true);
          BOOST_FOREACH (const value_type& x, v)
            {
              if (!first)
                {
                  _os << ", ";
                }
              boost::apply_visitor (*this, x);
              first = false;
            }
          return _os << ")";
        }
        std::ostream& operator() (const std::set<value_type>& s) const
        {
          _os << "set {";
          bool first (true);
          BOOST_FOREACH (const value_type& v, s)
            {
              if (!first)
                {
                  _os << ", ";
                }
              boost::apply_visitor (*this, v);
              first = false;
            }
          return _os << "}";
        }
        std::ostream& operator() (const structured_type& m) const
        {
          _os << "struct [";
          bool first (true);
          typedef std::pair<std::string, value_type> kv_type;
          BOOST_FOREACH (const kv_type& kv, m)
            {
              if (!first)
                {
                  _os << ", ";
                }
              _os << kv.first << " := ";
              boost::apply_visitor (*this, kv.second);
              first = false;
            }
          return _os << "]";
        }
      private:
        std::ostream& _os;
      };
    }
  }
}

std::ostream& operator<< ( std::ostream& os
                         , const pnet::type::value::value_type& v
                         )
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint << std::boolalpha;
  boost::apply_visitor (pnet::type::value::visitor_show (os), v);
  os.flags (ff);
  return os;
}
