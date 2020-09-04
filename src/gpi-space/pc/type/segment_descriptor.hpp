// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>
#include <gpi-space/pc/type/segment_type.hpp>

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
          gpi::pc::type::ref_count_t nref;      // number of containers attached
          gpi::pc::type::flags_t const flags;         // flags (see above)
          gpi::pc::type::time_stamp_t ts;       // time stamps

          descriptor_t ()
            : id (SEG_INVAL)
            , type (SEG_INVAL)
            , creator (0)
            , name ("")
            , local_size (0)
            , avail (0)
            , allocs (0)
            , nref (0)
            , flags (0)
          {}

          descriptor_t ( const gpi::pc::type::segment_id_t a_id
                       , const segment_type a_type
                       , const gpi::pc::type::process_id_t a_proc
                       , const gpi::pc::type::name_t a_name
                       , const gpi::pc::type::size_t a_size
                       , const gpi::pc::type::flags_t a_flags
                       )
              : id (a_id)
              , type (a_type)
              , creator (a_proc)
              , name (a_name)
              , local_size (a_size)
              , avail (a_size)
              , allocs (0)
              , nref (0)
              , flags (a_flags)
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
