// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <util-generic/first_then.hpp>

#include <functional>

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
      , const std::function<void (const typename C::value_type&)>& f
      )
    {
      os << header << open;

      const first_then<std::string> pre ("", sep + " ");
      for (const typename C::value_type& x : c)
      {
        pre (os);
        f (x);
      }

      return os << close;
    }
  }
}
