#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        struct descriptor_t
        {
          gpi::pc::type::handle_t id;
          gpi::pc::type::offset_t offset;
          gpi::pc::type::size_t size;
          gpi::pc::type::size_t local_size;
          gpi::pc::type::name_t name;
          gpi::pc::type::flags_t flags;

          descriptor_t ()
            : id (0)
            , offset (0)
            , size (0)
            , local_size (0)
            , name ("")
            , flags (is_global::no)
          {}

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id.handle );
            ar & BOOST_SERIALIZATION_NVP( offset );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP (local_size);
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( flags );
          }
        };
      }
    }
  }
}
