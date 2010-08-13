// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_SHOW_HPP
#define _WE_TYPE_LITERAL_SHOW_HPP

#include <fhg/util/show.hpp>

#include <we/type/literal.hpp>

#include <iostream>
#include <sstream>

#include <iomanip>

namespace literal
{
  namespace visitor
  {
    class show : public boost::static_visitor<std::string>
    {
    public:
      std::string operator () (const bool & x) const
      {
        return x ? "true" : "false";
      }

      std::string operator () (const long & x) const
      {
        return fhg::util::show (x) + "L";
      }

      std::string operator () (const char & x) const
      {
        return "'" + fhg::util::show (x) + "'";
      }

      std::string operator () (const std::string & x) const
      {
        return "\"" + x + "\"";
      }

      std::string operator () (const double & d) const
      {
        std::ostringstream s; s << std::showpoint << d; return s.str();
      }

      template<typename T>
      std::string operator () (const T & x) const
      {
        return fhg::util::show (x);
      }
    };
  }

  inline std::string show (const type & v)
  {
    return boost::apply_visitor (visitor::show(), v);
  }
}

#endif
