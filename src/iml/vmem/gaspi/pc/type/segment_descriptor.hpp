#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/segment_type.hpp>

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
          gpi::pc::type::segment_id_t id;       // identification
          segment_type type;                    // type id
          gpi::pc::type::size_t local_size;     // maximum local size

          descriptor_t ()
            : id (SEG_INVAL)
            , type (SEG_INVAL)
            , local_size (0)
          {}

          descriptor_t ( const gpi::pc::type::segment_id_t a_id
                       , const segment_type a_type
                       , const gpi::pc::type::size_t a_size
                       )
              : id (a_id)
              , type (a_type)
              , local_size (a_size)
          {}

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( type );
            ar & BOOST_SERIALIZATION_NVP( local_size );
          }
        };
      }
    }
  }
}
