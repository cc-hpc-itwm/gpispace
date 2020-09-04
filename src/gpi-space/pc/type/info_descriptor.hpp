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

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace info
      {
        struct descriptor_t
        {
          descriptor_t ()
            : rank (0)
            , nodes (0)
            , queues (0)
            , queue_depth (0)
          {}

          // gpi related information
          gpi::pc::type::size_t       rank;
          gpi::pc::type::size_t       nodes;
          gpi::pc::type::size_t       queues;
          gpi::pc::type::size_t       queue_depth;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( rank );
            ar & BOOST_SERIALIZATION_NVP( nodes );
            ar & BOOST_SERIALIZATION_NVP( queues );
            ar & BOOST_SERIALIZATION_NVP( queue_depth );
          }
        };
      }
    }
  }
}
