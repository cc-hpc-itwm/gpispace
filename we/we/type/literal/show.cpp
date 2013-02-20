// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/show.hpp>

#include <boost/foreach.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace literal
{
  namespace
  {
    class visitor_show : public boost::static_visitor<std::ostream&>
    {
    private:
      std::ostream& _os;

    public:
      visitor_show (std::ostream& os)
        : _os (os)
      {}

      std::ostream& operator() (const we::type::literal::control& c) const
      {
        return _os << c;
      }

      std::ostream& operator() (const bool& x) const
      {
        return _os << (x ? "true" : "false");
      }

      std::ostream& operator() (const long& x) const
      {
        return _os << x << "L";
      }

      std::ostream& operator() (const double& d) const
      {
        return _os << std::showpoint << d;
      }

      std::ostream& operator() (const char& x) const
      {
        return _os << "'" << x << "'";
      }

      std::ostream& operator() (const std::string& x) const
      {
        return _os << "\"" << x << "\"";
      }

      std::ostream& operator() (const bitsetofint::type& bs) const
      {
        return _os << bs;
      }

      std::ostream& operator() (const literal::stack_type& stack) const
      {
        return _os << stack;
      }

      std::ostream& operator() (const literal::map_type& map) const
      {
        return _os << map;
      }

      std::ostream& operator() (const literal::set_type& set) const
      {
        return _os << set;
      }

      std::ostream& operator() (const bytearray::type& ba) const
      {
        return _os << ba;
      }
    };
  }

  std::ostream& operator<< (std::ostream& os, const type& v)
  {
    return boost::apply_visitor (visitor_show (os), v);
  }
}

namespace std
{
  std::ostream& operator<< (std::ostream& os, const literal::stack_type& s)
  {
    os << "@";

    literal::stack_type c (s);

    while (!c.empty())
    {
      os << " " << c.back(); c.pop_back();
    }

    return os << "@";
  }

  std::ostream& operator<< (std::ostream& os, const literal::map_type& m)
  {
    os << "{|";

    for ( literal::map_type::const_iterator pos (m.begin())
        ; pos != m.end()
        ; ++pos
        )
    {
      os << (pos != m.begin() ? ", " : "")
          << pos->first << " -> " << pos->second
        ;
    }

    return os << "|}";
  }

  std::ostream& operator<< (std::ostream& os, const literal::set_type& s)
  {
    os << "{:";

    BOOST_FOREACH (const long& v, s)
    {
      os << " " << v;
    }

    return os << ":}";
  }
}
