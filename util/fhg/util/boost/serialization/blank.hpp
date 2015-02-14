// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/blank.hpp>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void serialize (Archive&, boost::blank&, const unsigned int)
    {
    }
  }
}
