#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/time_stamp.hpp>
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
          gpi::pc::type::process_id_t creator;  // container id
          gpi::pc::type::name_t name;           // user defined name
          gpi::pc::type::size_t local_size;     // maximum local size
          gpi::pc::type::size_t avail;          // available size
          gpi::pc::type::size_t allocs;         // number of allocations
          gpi::pc::type::time_stamp_t ts;       // time stamps

          descriptor_t ()
            : id (SEG_INVAL)
            , type (SEG_INVAL)
            , creator (0)
            , name ("")
            , local_size (0)
            , avail (0)
            , allocs (0)
          {}

          descriptor_t ( const gpi::pc::type::segment_id_t a_id
                       , const segment_type a_type
                       , const gpi::pc::type::process_id_t a_proc
                       , const gpi::pc::type::name_t a_name
                       , const gpi::pc::type::size_t a_size
                       )
              : id (a_id)
              , type (a_type)
              , creator (a_proc)
              , name (a_name)
              , local_size (a_size)
              , avail (a_size)
              , allocs (0)
          {}

          bool operator< (const descriptor_t & other) const
          {
            return id < other.id;
          }

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( type );
            ar & BOOST_SERIALIZATION_NVP( creator );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( local_size );
            ar & BOOST_SERIALIZATION_NVP( avail );
            ar & BOOST_SERIALIZATION_NVP( allocs );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;
      }
    }
  }
}
