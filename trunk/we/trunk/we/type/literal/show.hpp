// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_SHOW_HPP
#define _WE_TYPE_LITERAL_SHOW_HPP

#include <we/util/show.hpp>

#include <we/type/literal.hpp>

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
        return util::show (x) + "L";
      }

      std::string operator () (const char & x) const
      {
        return "'" + util::show (x) + "'";
      }

      std::string operator () (const std::string & x) const
      {
        return "\"" + x + "\"";     
      }

      template<typename T>
      std::string operator () (const T & x) const
      {
        return util::show (x);
      }
    };
  }

  static std::string show (const type v)
  {
    return boost::apply_visitor (visitor::show(), v);
  }
}

#endif
