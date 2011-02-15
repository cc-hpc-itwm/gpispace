#ifndef GPI_SPACE_PC_TYPE_MEMORY_LOCATION_HPP
#define GPI_SPACE_PC_TYPE_MEMORY_LOCATION_HPP 1

// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct memory_location_t
      {
        gpi::pc::type::handle_id_t handle;
        gpi::pc::type::offset_t    offset;

      private:
        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive & ar, const unsigned int /*version*/)
        {
          ar & BOOST_SERIALIZATION_NVP( handle );
          ar & BOOST_SERIALIZATION_NVP( offset );
        }
      };
    }
  }
}

#endif
