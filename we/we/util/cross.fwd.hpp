// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_UTIL_CROSS_FWD_HPP
#define _WE_UTIL_CROSS_FWD_HPP

#include <we/type/id.hpp>
#include <we/type/token.hpp>

#include <vector>

namespace we
{
  namespace util
  {
    class cross_type;

    typedef std::pair<petri_net::place_id_type,token::type> token_on_place_type;
    typedef std::vector<token_on_place_type> tokens_on_places_type;
  }
}

#endif
