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

#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <atomic>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_generator_t : boost::noncopyable
      {
      public:
        /** create a new handle_generator for a given node identifier.

            usually the rank can be used
         */

        explicit
        handle_generator_t (const gpi::pc::type::size_t identifier);

        /*
          generate a new handle for the given segment

          the handle ids are generated as follows:
          the four most significant bits indicate:

              0x0     - invalid alloc
              0x1     - gpi allocation
              0x2     - shared memory
              0x3     - gpu allocation
              0x4-0xf - reserved

          the remaining bits are assigned in an implementation specific way.
        */
        gpi::pc::type::handle_t
        next (const gpi::pc::type::segment::segment_type);

        void initialize_counter (const gpi::pc::type::segment::segment_type);
      private:
        gpi::pc::type::size_t m_node_identifier;
        std::vector<std::atomic<gpi::pc::type::size_t>> m_counter;
      };
    }
  }
}
