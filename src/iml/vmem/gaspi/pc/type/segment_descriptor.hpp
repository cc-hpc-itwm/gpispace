#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace segment
      {
        struct descriptor_t
        {
          gpi::pc::type::size_t local_size;     // maximum local size

          descriptor_t ()
            : local_size (0)
          {}

          descriptor_t ( const gpi::pc::type::size_t a_size
                       )
              : local_size (a_size)
          {}

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( local_size );
          }
        };
      }
    }
  }
}
