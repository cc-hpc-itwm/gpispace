#include <iostream>
#include <boost/foreach.hpp>

#include "frame_io.hpp"

#include "frame.hpp"

namespace gspc
{
  namespace net
  {
    std::ostream & operator<< ( std::ostream & os
                              , frame const & f
                              )
    {
      os << f.get_command () << std::endl;
      BOOST_FOREACH ( frame::header_type::value_type const & kvp
                    , f.get_header ()
                    )
      {
        os << kvp.first << ":" << kvp.second << std::endl;
      }
      os << std::endl;
      os << f.get_body_as_string ();
      return os;
    }
  }
}
