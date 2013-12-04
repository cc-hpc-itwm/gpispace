// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_PRINT_CONTAINER_HPP
#define FHG_UTIL_PRINT_CONTAINER_HPP

#include <fhg/util/first_then.hpp>

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

      const first_then<std::string> pre ("", sep + " ");
      BOOST_FOREACH (const typename C::value_type& x, c)
      {
        pre (os);
        f (x);
      }

      return os << close;
    }
  }
}

#endif
