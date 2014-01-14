// bernd.loerwald@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_FWD_HPP
#define _WE_TYPE_PROPERTY_FWD_HPP

#include <we/type/value.hpp>

#include <string>
#include <list>

#include <boost/unordered/unordered_map_fwd.hpp>
#include <boost/variant/variant_fwd.hpp>
#include <boost/variant/recursive_wrapper_fwd.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef std::string key_type;
      typedef std::string value_type;

      struct type;

      typedef pnet::type::value::value_type mapped_type;

      typedef std::list<key_type> path_type;
      typedef std::list<std::pair<key_type, mapped_type> > list_type;

      namespace exception
      {
        class missing_binding;
        class empty_path;
      }
    }
  }
}

#endif
