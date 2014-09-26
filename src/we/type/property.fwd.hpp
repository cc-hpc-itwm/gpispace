// bernd.loerwald@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_FWD_HPP
#define _WE_TYPE_PROPERTY_FWD_HPP

#include <we/type/value.hpp>

#include <string>
#include <list>

#include <boost/variant/variant_fwd.hpp>
#include <boost/variant/recursive_wrapper_fwd.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef std::string key_type;
      typedef pnet::type::value::value_type value_type;

      struct type;

      typedef std::list<key_type> path_type;
    }
  }
}

#endif
