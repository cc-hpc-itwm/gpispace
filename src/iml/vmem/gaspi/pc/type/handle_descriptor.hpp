// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

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
          iml::AllocationHandle id;
          iml::MemoryOffset offset;
          iml::MemorySize size;
          iml::MemorySize local_size;
          gpi::pc::type::flags_t flags;

          descriptor_t ()
            : id()
            , offset (0)
            , size (0)
            , local_size (0)
            , flags (is_global::no)
          {}

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( offset );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP (local_size);
            ar & BOOST_SERIALIZATION_NVP( flags );
          }
        };
      }
    }
  }
}
