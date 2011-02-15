#ifndef GPI_SPACE_PC_TYPE_TIMESTAMP_HPP
#define GPI_SPACE_PC_TYPE_TIMESTAMP_HPP 1

// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct time_stamp_t
      {
        gpi::pc::type::time_t created;
        gpi::pc::type::time_t modified;
        gpi::pc::type::time_t accessed;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( created );
            ar & BOOST_SERIALIZATION_NVP( modified );
            ar & BOOST_SERIALIZATION_NVP( accessed );
          }
      };
    }
  }
}

#endif
