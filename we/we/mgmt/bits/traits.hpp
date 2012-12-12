// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_BITS_TRAITS_HPP
#define WE_MGMT_BITS_TRAITS_HPP 1

#include <string>
#include <limits>

#include <we/type/id.hpp>
#include <we/util/codec.hpp>

namespace we
{
  namespace mgmt
  {
    namespace traits
    {
      namespace def
      {
        template <typename IdType>
        struct id_traits
        {
          typedef IdType type;
        };

        template <>
        struct id_traits<petri_net::activity_id_type>
        {
        public:
          typedef petri_net::activity_id_type type;

          inline static type generate()
          {
            static type _id = init();
            return _id++;
          }
        private:
          inline static type init()
          {
            return 0;
          }
        };
      }

      struct layer_traits
      {
        typedef def::id_traits<petri_net::activity_id_type> id_traits;
      };
    }
  }
}

#endif
