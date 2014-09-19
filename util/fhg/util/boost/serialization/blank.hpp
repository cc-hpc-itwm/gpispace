// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_SERIALIZATION_BLANK_HPP
#define FHG_UTIL_BOOST_SERIALIZATION_BLANK_HPP

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

#endif
