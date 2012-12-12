// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_BITS_TRAITS_HPP
#define WE_MGMT_BITS_TRAITS_HPP 1

#include <we/type/id.hpp>

namespace we
{
  namespace mgmt
  {
    namespace traits
    {
      namespace id_traits
      {
        inline static petri_net::activity_id_type generate()
        {
          static  petri_net::activity_id_type _id (0);
          return _id++;
        }
      }
    }
  }
}

#endif
