// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_UTIL_CROSS_FWD_HPP
#define _WE_UTIL_CROSS_FWD_HPP

#include <we/type/value.hpp>
#include <we2/type/value.hpp>

#include <cstddef>

namespace we
{
  namespace util
  {
    typedef std::pair< std::list<pnet::type::value::value_type>::iterator
                     , std::size_t
                     > pos_and_distance_type;

    class cross_type;
  }
}

#endif
