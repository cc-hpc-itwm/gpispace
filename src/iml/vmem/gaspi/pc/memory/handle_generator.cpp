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

#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

#include <climits>
#include <cstdint>
#include <stdexcept>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      handle_generator_t::handle_generator_t (std::size_t identifier)
        : m_node_identifier (identifier)
        , m_counter (0)
      {}

      namespace
      {
        template <std::size_t Bits, typename Integer>
          void check_for_overflow (Integer part)
        {
          static_assert ( Bits < sizeof (Integer) * CHAR_BIT
                        , "check would overflow itself"
                        );
          if (part > ((Integer (1) << Bits) - 1))
          {
            throw std::runtime_error ("handle would overflow");
          }
        }

        //! In order to save on bits we pack the allocating rank as
        //! well as a counter of all segments/segments on that rank
        //! into 64 bits. We give 12 bits to the ranks (so max number
        //! of ranks is 4096) and the remaining bits for the counter.
        // \todo Just use two variables in the handles? 32bit each
        // should also be enough. Or just use 2x64bit.
        std::uint64_t bitpack (std::uint64_t global_id, std::uint64_t counter)
        {
          static constexpr const auto ident_bits = 12;
          static constexpr const auto cntr_bits = 64 - ident_bits;

          check_for_overflow<ident_bits> (global_id);
          check_for_overflow<cntr_bits> (counter);

          return (global_id << cntr_bits) | counter;
        }
      }

      iml::AllocationHandle handle_generator_t::next_allocation()
      {
        return iml::AllocationHandle (bitpack (m_node_identifier, ++m_counter));
      }

      iml::SegmentHandle handle_generator_t::next_segment()
      {
        return iml::SegmentHandle (bitpack (m_node_identifier, ++m_counter));
      }
    }
  }
}
