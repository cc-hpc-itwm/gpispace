#ifndef GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP 1

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
      namespace segment
      {
        enum segment_type
          {
            SEG_INVAL = 0,
            SEG_GLOBAL,
            SEG_LOCAL,
            // the following is just a placeholder
            // shared segment ids are just >= SHARED
            SEG_SHARED,
          };

        enum flags_type
          {
            F_NONE       = 0x00,
            F_PERSISTENT = 0x01,
            F_EXCLUSIVE  = 0x02,
          };

        struct traits
        {
          static bool is_valid (const gpi::pc::type::segment_id_t i)
          {
            return i != SEG_INVAL;
          }

          static bool is_global (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_GLOBAL;
          }
          static bool is_local (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_LOCAL;
          }
          static bool is_shared (const gpi::pc::type::segment_id_t i)
          {
            return i >= SEG_SHARED;
          }
        };

        struct descriptor_t
        {
          typedef traits traits_type;
          gpi::pc::type::segment_id_t id;
          gpi::pc::type::name_t name;
          gpi::pc::type::ref_count_t nref;
          gpi::pc::type::mode_t flags;
          gpi::pc::type::time_stamp_t ts;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( nref );
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
