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
      os << f.to_string ();
      return os;
    }
  }
}
