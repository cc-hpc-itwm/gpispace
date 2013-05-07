// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_PRINT_CONTAINER_HPP
#define FHG_UTIL_PRINT_CONTAINER_HPP

#include <boost/foreach.hpp>
#include <boost/function.hpp>

namespace fhg
{
  namespace util
  {
    template<typename C>
    inline std::ostream& print_container
      ( std::ostream& os
      , const std::string& header
      , const std::string& open
      , const std::string& sep
      , const std::string& close
      , const C& c
      , const boost::function<void (const typename C::value_type&)>& f
      )
    {
      os << header << open;
      bool first (true);
      BOOST_FOREACH (const typename C::value_type& x, c)
      {
        if (!first)
        {
          os << sep << " ";
        }
        f (x);
        first = false;
      }
      return os << close;
    }
  }
}

#endif
