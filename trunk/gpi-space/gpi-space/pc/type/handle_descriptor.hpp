#ifndef GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP 1

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        struct traits
        {
          static bool is_null (const gpi::pc::type::handle_id_t i)
          {
            return 0 == i;
          }
        };

        struct descriptor_t
        {
          gpi::pc::type::handle_id_t id;
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::offset_t offset;
          gpi::pc::type::size_t size;
          gpi::pc::type::name_t name;
          gpi::pc::type::mode_t flags;
          gpi::pc::type::time_stamp_t ts;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( segment );
            ar & BOOST_SERIALIZATION_NVP( offset );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( flags );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;
      }
    }
  }
}

#endif
