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

      std::ostream& operator() (const bool& x) const
      {
        return _os << (x ? "true" : "false");
      }

      std::ostream& operator() (const long& x) const
      {
        return _os << x << "L";
      }

      std::ostream& operator() (const char& x) const
      {
        return _os << "'" << x << "'";
      }

      std::ostream& operator() (const std::string& x) const
      {
        return _os << "\"" << x << "\"";
      }

      std::ostream& operator() (const double& d) const
      {
        return _os << std::showpoint << d;
      }

      std::ostream& operator() (const literal::stack_type& stack) const
      {
        _os << "@";

        literal::stack_type c (stack);

        while (!c.empty())
          {
            _os << " " << c.back(); c.pop_back();
          }

        return _os << "@";
      }

      std::ostream& operator() (const literal::map_type& map) const
      {
        _os << "{|";

        for ( literal::map_type::const_iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
          {
            _os << (pos != map.begin() ? ", " : "")
                << pos->first << " -> " << pos->second
              ;
          }

        return _os << "|}";
      }

      std::ostream& operator() (const literal::set_type& set) const
      {
        _os << "{:";

        BOOST_FOREACH (const long& v, set)
          {
            _os << " " << v;
          }

        return _os << ":}";
      }

      template<typename T>
      std::ostream& operator() (const T& x) const
      {
        return _os << x;
      }
    };
  }

  std::string show (const type& v)
  {
    std::ostringstream oss;
    boost::apply_visitor (visitor_show (oss), v);
    return oss.str();
  }
}
