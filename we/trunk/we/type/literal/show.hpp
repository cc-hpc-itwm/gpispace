// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_SHOW_HPP
#define _WE_TYPE_LITERAL_SHOW_HPP

#include <we/type/literal.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <iomanip>

namespace literal
{
  namespace visitor
  {
    namespace detail
    {
      template<typename T>
      inline std::string show (const T & x)
      {
        std::ostringstream s; s << x; return s.str();
      }

      template<>
      inline std::string show<std::string> (const std::string & x)
      {
        return x;
      }
    }

    class show : public boost::static_visitor<std::string>
    {
    public:
      std::string operator () (const bool & x) const
      {
        return x ? "true" : "false";
      }

      std::string operator () (const long & x) const
      {
        return detail::show (x) + "L";
      }

      std::string operator () (const char & x) const
      {
        return "'" + detail::show (x) + "'";
      }

      std::string operator () (const std::string & x) const
      {
        return "\"" + x + "\"";
      }

      std::string operator () (const double & d) const
      {
        std::ostringstream s; s << std::showpoint << d; return s.str();
      }

      std::string operator () (const literal::stack_type & stack) const
      {
        std::ostringstream s;

        s << "@";

        literal::stack_type c (stack);

        while (!c.empty())
          {
            s << " " << c.back(); c.pop_back();
          }

        s << "@";

        return s.str();
      }

      std::string operator () (const literal::map_type & map) const
      {
        std::ostringstream s;

        s << "[|";

        for ( literal::map_type::const_iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
          {
            s << (pos != map.begin() ? ", " : "")
              << pos->first << " -> " << pos->second
              ;
          }

        s << "|]";

        return s.str();
      }

      std::string operator () (const literal::set_type & set) const
      {
        std::ostringstream s;

        s << "{:";

        for ( literal::set_type::const_iterator pos (set.begin())
            ; pos != set.end()
            ; ++pos
            )
          {
            s << " " << *pos;
          }

        s << ":}";

        return s.str();
      }

      template<typename T>
      std::string operator () (const T & x) const
      {
        return detail::show (x);
      }
    };
  }

  inline std::string show (const type & v)
  {
    return boost::apply_visitor (visitor::show(), v);
  }
}

#endif
